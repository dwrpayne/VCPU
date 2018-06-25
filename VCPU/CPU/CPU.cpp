#include <vector>
#include <future>
#include <windows.h>

#include "CPU.h"
#include "Register.h"
#include "Memory.h"
#include "ALU.h"
#include "Multiplier.h"
#include "MuxBundle.h"
#include "BranchControl.h"
#include "ByteExtractor.h"
#include "SystemBus.h"

class CPU::Stage1 : public ThreadedAsyncComponent
{
public:
	using ThreadedAsyncComponent::ThreadedAsyncComponent;
	void Connect(const Bundle<32>& pcBranchAddr, const Wire& takeBranch, const Wire& proceed, SystemBus& systembus);
	void Update();
	void PostUpdate();
	const BufferIFID& Out() const { return bufIFID; }
private:
	MuxBundle<32, 2> pcBranchInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
	CPU::InsCache instructionCache;
	BufferIFID bufIFID;

	friend class CPU;
};

class CPU::Stage2 : public ThreadedAsyncComponent
{
public:
	using ThreadedAsyncComponent::ThreadedAsyncComponent;
	void Connect(const BufferIFID& stage1, const BufferMEMWB& stage4, const HazardUnit& hazard, const Wire& proceed, const Wire& flush);
	void Update();
	void PostUpdate();
	const BufferIDEX& Out() const { return bufIDEX; }
private:
	OpcodeDecoder opcodeControl;
	CPU::RegFile regFile;
	MuxBundle<32, 2> reg2ShiftMux;
	Extender<16, 32> immExtender;
	MuxBundle<32, 2> immShiftMux;
	
	MuxBundle<32, 2> rsForwardMux;
	MuxBundle<32, 2> rtForwardMux;
	BranchControl branchControl;
	OrGate branchTaken;
	MuxBundle<CPU::RegFile::ADDR_BITS, 2> regWriteAddrMux;
	BufferIDEX bufIDEX;

	friend class CPU;
};

class CPU::Stage3 : public ThreadedAsyncComponent
{
public:
	using ThreadedAsyncComponent::ThreadedAsyncComponent;
	void Connect(const BufferIDEX& stage2, const HazardUnit& hazard, const Wire& proceed, const Wire& flush);
	void Update();
	void PostUpdate();
	const BufferEXMEM& Out() const { return bufEXMEM; }
private:
	MuxBundle<CPU::RegFile::ADDR_BITS, 2> regFileWriteAddrMux;

	MuxBundle<32, 2> aluAInputMux;
	MuxBundle<32, 2> aluBForwardMux;
	MuxBundle<32, 2> aluBInputMux;

	ALU<32> alu;
	Multiplier<32> multiplier;
	MuxBundle<32, 4> aluMultOutMux;
	MuxBundle<32, 4> aluOutMux;
	FullAdderN<32> jumpLinkAdder;
	BufferEXMEM bufEXMEM;

	friend class CPU;
};

class CPU::Stage4 : public ThreadedAsyncComponent
{
public:
	using ThreadedAsyncComponent::ThreadedAsyncComponent;
	void Connect(const BufferEXMEM& stage3, const Wire& proceed, SystemBus& systembus);
	void Update();
	void PostUpdate();
	const BufferMEMWB& Out() const { return bufMEMWB; }
private:
	CPU::MainCache cache;
	ByteExtractor byteSelect;
	MuxBundle<32, 2> regWriteDataMux;
	BufferMEMWB bufMEMWB;

	friend class CPU;
};


void CPU::Stage1::Connect(const Bundle<32>& pcBranchAddr, const Wire& takeBranch, const Wire& proceed, SystemBus& systembus)
{
	// Program Counter. Select between PC + 4 and the calculated jump addr from the last executed branch
	pcBranchInMux.Connect({ bufIFID.PCinc.Out(), pcBranchAddr }, takeBranch);

	pc.Connect(pcBranchInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	pcIncrementer.Connect(pc.Out(), Bundle<32>(4), Wire::OFF);

	// Instruction memory
	instructionCache.Connect(pc.Out().Range<InsCache::ADDR_BITS>(0), InsCache::DataBundle::OFF, Wire::ON, Wire::OFF, Wire::OFF, Wire::OFF, systembus);

	// Out Buffer
	bufIFID.Connect(proceed, instructionCache.Out(), pcIncrementer.Out());
}

void CPU::Stage2::Connect(const BufferIFID& stage1, const BufferMEMWB& stage4, const HazardUnit& hazard, const Wire& proceed, const Wire& flush)
{
	// Opcode Decoding
	opcodeControl.Connect(stage1.IR.Opcode(), stage1.IR.Function());

	// Register File
	regFile.Connect(stage1.IR.RsAddr(), stage1.IR.RtAddr(),
		stage4.Rwrite.Out(), stage4.RWriteData.Out(),
		stage4.OpcodeControl().RegWrite());

	// Shift Amount
	reg2ShiftMux.Connect({ regFile.Out2(), stage1.IR.Shamt().ZeroExtend<32>() },
		opcodeControl.OutBundle().ShiftAmtOp());

	// Immediate Operand
	immExtender.Connect(stage1.IR.Immediate(), opcodeControl.OutBundle().MathOp());
	immShiftMux.Connect({ immExtender.Out(), stage1.IR.Immediate().ShiftZeroExtend<32>(16) },
		opcodeControl.OutBundle().LoadUpperImm());

	// rs/rt forward
	rsForwardMux.Connect({ regFile.Out1(), hazard.ForwardDataRs() }, hazard.DoForwardRs());
	rtForwardMux.Connect({ regFile.Out2(), hazard.ForwardDataRt() }, hazard.DoForwardRt());

	// Branch Controller
	branchControl.Connect(rsForwardMux.Out(), rtForwardMux.Out(), stage1.PCinc.Out(), stage1.IR.Immediate(),
		opcodeControl.OutBundle().Branch(), opcodeControl.OutBundle().BranchSel(),
		stage1.IR.Address().ZeroExtend<32>(), opcodeControl.OutBundle().JumpReg());

	branchTaken.Connect(branchControl.BranchTaken(), opcodeControl.OutBundle().JumpOp());

	regWriteAddrMux.Connect({ stage1.IR.RtAddr(), stage1.IR.RdAddr() }, opcodeControl.OutBundle().RFormat());
	
	// Out Buffer
	bufIDEX.Connect(proceed, flush, stage1.IR.RsAddr(), stage1.IR.RtAddr(), regWriteAddrMux.Out(),
		immShiftMux.Out(), regFile.Out1(), reg2ShiftMux.Out(), branchControl.NewPC(),
		stage1.PCinc.Out(), stage1.IR.Opcode(),
		opcodeControl.OutBundle(), opcodeControl.AluControl(), branchTaken.Out());
}

void CPU::Stage3::Connect(const BufferIDEX& stage2, const HazardUnit& hazard, const Wire& proceed, const Wire& flush)
{
	// Hardcoded Link in R31.
	regFileWriteAddrMux.Connect({ stage2.RD.Out(), Bundle<5>(31U) }, stage2.OpcodeControl().JumpLink());

	// ALU Input A
	aluAInputMux.Connect({ stage2.reg1.Out(), hazard.ForwardDataRs() }, hazard.DoForwardRs());

	// ALU Input B
	aluBForwardMux.Connect({ stage2.reg2.Out(), hazard.ForwardDataRt() }, hazard.DoForwardRt());
	aluBInputMux.Connect({ aluBForwardMux.Out(), stage2.immExt.Out() }, stage2.OpcodeControl().AluBFromImm());

	// ALU
	alu.Connect(aluAInputMux.Out(), aluBInputMux.Out(), stage2.aluControl.Out());

	// SLT instruction
	Bundle<32> sltExtended = Bundle<32>::OFF;
	sltExtended.Connect(0, alu.Flags().Negative());

	// Multiplier
	multiplier.Connect(aluAInputMux.Out(), aluBInputMux.Out(), stage2.OpcodeControl().MultOp());
	aluMultOutMux.Connect({ alu.Out(), alu.Out(), multiplier.OutHi(), multiplier.OutLo() }, stage2.OpcodeControl().MultMoveReg());

	// Bit of a hack? Drop the PC+4+4 into the alu out field for Jump Link instructions
	jumpLinkAdder.Connect(stage2.PCinc.Out(), Bundle<32>(4), Wire::OFF);
	aluOutMux.Connect({ aluMultOutMux.Out(), jumpLinkAdder.Out(), sltExtended, sltExtended },
		{ &stage2.OpcodeControl().JumpLink(), &stage2.OpcodeControl().SltOp() });
	
	// Out Buffer
	bufEXMEM.Connect(proceed, flush, regFileWriteAddrMux.Out(), aluBForwardMux.Out(), aluOutMux.Out(),
		alu.Flags(), stage2.OpcodeControl());
}

void CPU::Stage4::Connect(const BufferEXMEM& stage3, const Wire& proceed, SystemBus& systembus)
{
	// Main Memory
	const auto& memAddr = stage3.aluOut.Out();
	cache.Connect(memAddr.Range<MainCache::ADDR_BITS>(0), stage3.reg2.Out(), stage3.OpcodeControl().LoadOp(),
		stage3.OpcodeControl().StoreOp(), stage3.OpcodeControl().MemOpByte(), stage3.OpcodeControl().MemOpHalfWord(), systembus);


	// Byte/Half/Word Selection
	byteSelect.Connect(cache.Out(), memAddr.Range<2>(0), stage3.OpcodeControl().LoadSigned(),
		stage3.OpcodeControl().MemOpByte(), stage3.OpcodeControl().MemOpHalfWord());

	// Regfile Data Write
	regWriteDataMux.Connect({ stage3.aluOut.Out(), byteSelect.Out() }, stage3.OpcodeControl().LoadOp());
	
	// Out Buffer
	bufMEMWB.Connect(proceed, stage3.Rwrite.Out(), regWriteDataMux.Out(), stage3.OpcodeControl());
}


// ******** STAGE 1 INSTRUCTION FETCH ************
void CPU::Stage1::Update()
{
	pcBranchInMux.Update();
	pc.Update();
	pcIncrementer.Update();
	instructionCache.Update();
}
void CPU::Stage1::PostUpdate()
{
	bufIFID.Update();
}


// ******** STAGE 2 INSTRUCTION DECODE ************
void CPU::Stage2::Update()
{
	opcodeControl.Update();
	regFile.Update();
	rsForwardMux.Update();
	rtForwardMux.Update();
	reg2ShiftMux.Update();
	immExtender.Update();
	immShiftMux.Update();
	branchControl.Update();
	branchTaken.Update();
	regWriteAddrMux.Update();
}
void CPU::Stage2::PostUpdate()
{
	bufIDEX.Update();
}


// ******** STAGE 3 EXECUTION ************
void CPU::Stage3::Update()
{
	regFileWriteAddrMux.Update();
	aluAInputMux.Update();
	aluBForwardMux.Update();
	aluBInputMux.Update();
	alu.Update();
	multiplier.Update();
	aluMultOutMux.Update();
	jumpLinkAdder.Update();
	aluOutMux.Update();
}
void CPU::Stage3::PostUpdate()
{
	bufEXMEM.Update();
}


// ******** STAGE 4 BEGIN - MEMORY STORE ************
void CPU::Stage4::Update()
{
	cache.Update();
	byteSelect.Update();
	regWriteDataMux.Update();
}
void CPU::Stage4::PostUpdate()
{
	bufMEMWB.Update();
}


CPU::CPU()
	: exit(false)
	, stage1(new Stage1(L"CPU Stage 1 Instruction Fetch"))
	, stage2(new Stage2(L"CPU Stage 2 Instruction Decode"))
	, stage3(new Stage3(L"CPU Stage 3 Execution"))
	, stage4(new Stage4(L"CPU Stage 4 Memory Access"))
	, mInsMemory(new InsMemory(false))
	, mMainMemory(new MainMemory(true))
	, cycles(1)
{
	hazardIFID.Connect(stage3->Out().Rwrite.Out(), stage3->Out().aluOut.Out(), stage3->Out().OpcodeControl().RegWrite(),
		stage4->Out().Rwrite.Out(), stage4->Out().RWriteData.Out(), stage4->Out().OpcodeControl().RegWrite(),
		stage1->Out().IR.RsAddr(), stage1->Out().IR.RtAddr(), stage3->Out().OpcodeControl().LoadOpInv());
	hazardIDEX.Connect(stage3->Out().Rwrite.Out(), stage3->Out().aluOut.Out(), stage3->Out().OpcodeControl().RegWrite(),
		stage4->Out().Rwrite.Out(), stage4->Out().RWriteData.Out(), stage4->Out().OpcodeControl().RegWrite(),
		stage2->Out().RS.Out(), stage2->Out().RT.Out(), Wire::ON);

	interlock.Connect(InstructionCache().NeedStall(), GetMainCache().NeedStall(), 
		stage1->Out().IR.RsAddr(), stage1->Out().IR.RtAddr(), stage2->Out().RD.Out(), stage1->Out().IR.Opcode(),
		stage2->Out().RS.Out(), stage2->Out().RT.Out(),	stage3->Out().Rwrite.Out(), stage3->Out().OpcodeControl().LoadOp());

	mInsMemory->Connect(systemBus);
	mMainMemory->Connect(systemBus);
	mKeyboard.Connect(systemBus);
	mTerminal.Connect(systemBus);
}

CPU::~CPU()
{
	stage1->WaitUntilDone();
	stage2->WaitUntilDone();
	stage3->WaitUntilDone();
	stage4->WaitUntilDone();
	mInsMemory->WaitUntilDone();
	mMainMemory->WaitUntilDone();
	mKeyboard.WaitUntilDone();
	mTerminal.WaitUntilDone();
	delete stage1;
	delete stage2;
	delete stage3;
	delete stage4;
	delete mInsMemory;
	delete mMainMemory;
}

void CPU::Connect()
{
	stage1->Connect(stage2->Out().pcJumpAddr.Out(), stage2->Out().branchTaken.Out(), interlock.ProceedIF(), systemBus);
	stage2->Connect(stage1->Out(), stage4->Out(), hazardIFID, interlock.ProceedID(), interlock.BubbleID());
	stage3->Connect(stage2->Out(), hazardIDEX, interlock.ProceedEX(), interlock.BubbleEX());
	stage4->Connect(stage3->Out(), interlock.ProceedMEM(), systemBus); 
}

void CPU::Update()
{
	stage1->DoOneUpdate();
	stage2->DoOneUpdate();
	stage3->DoOneUpdate();
	stage4->DoOneUpdate();

	stage1->WaitUntilDone();
	stage2->WaitUntilDone();
	stage3->WaitUntilDone();
	stage4->WaitUntilDone();
	
	interlock.Update();

	stage4->PostUpdate();
	stage3->PostUpdate();
	stage2->PostUpdate();
	stage1->PostUpdate();

	hazardIFID.Update();
	hazardIDEX.Update();
		
	mInsMemory->DoOneUpdate();
	mMainMemory->DoOneUpdate();
	mKeyboard.DoOneUpdate();
	mTerminal.DoOneUpdate();

	if (!PipelineFreeze() && !PipelineBubbleID() && !PipelineBubbleEX())
	{
		cycles++;
	}
}

const Bundle<32>& CPU::IR()
{
	return stage1->bufIFID.IR.Out();
}

const Bundle<32>& CPU::PC()
{
	return stage1->pc.Out();
}

CPU::InsCache& CPU::InstructionCache()
{
	return stage1->instructionCache;
}

CPU::MainCache& CPU::GetMainCache()
{
	return stage4->cache;
}

CPU::InsMemory& CPU::InstructionMemory()
{
	return *mInsMemory;
}

CPU::MainMemory& CPU::GetMainMemory()
{
	return *mMainMemory;
}

SystemBus & CPU::GetSystemBus()
{
	return systemBus;
}

CPU::RegFile& CPU::Registers()
{
	return stage2->regFile;
}

bool CPU::Halt() 
{ 
	// TODO: This should implement a pipeline freeze that can only be 
	// woken up by an interrupt. We are hijacking this to stop the debugger.
	return stage2->Out().OpcodeControl().Halt().On();
}

std::array<std::chrono::microseconds, 4> CPU::GetStageTiming()
{
	return {stage1->GetElapsedTime() / cycles,
		stage2->GetElapsedTime() / cycles,
		stage3->GetElapsedTime() / cycles,
		stage4->GetElapsedTime() / cycles
		};
}
