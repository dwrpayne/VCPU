#include "CPU.h"
#include "Register.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"
#include "SubWordSelector.h"
#include <vector>
#include <future>


class CPU::Stage1 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect(const Bundle<32>& pcBranchAddr, const Wire& takeBranch, const Bundle<32>& pcJumpAddr, const Wire& takeJump, const Wire& proceed);
	void Update();
	void PostUpdate();
	const BufferIFID& Out() const { return bufIFID; }
private:
	MuxBundle<32, 2> pcBranchInMux;
	MuxBundle<32, 2> pcJumpInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
	CPU::InsCache instructionCache;
	BufferIFID bufIFID;

	friend class CPU;
};

class CPU::Stage2 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect(const BufferIFID& stage1, const BufferMEMWB& stage4, const Wire& proceed);
	void Update();
	void PostUpdate();
	const BufferIDEX& Out() const { return bufIDEX; }
private:
	OpcodeDecoder opcodeControl;
	CPU::RegFile regFile;
	MuxBundle<32, 2> reg2ShiftMux;
	MuxBundle<32, 2> jumpAddrMux;
	Extender<16, 32> immExtender;
	MuxBundle<32, 2> immShiftMux;
	BufferIDEX bufIDEX;

	friend class CPU;
};

class CPU::Stage3 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect(const BufferIDEX& stage2, const HazardUnit& hazard, const Wire& proceed);
	void Update();
	void PostUpdate();
	const BufferEXMEM& Out() const { return bufEXMEM; }
private:
	MuxBundle<CPU::RegFile::ADDR_BITS, 4> regFileWriteAddrMux;

	MuxBundle<32, 4> aluAInputMux;
	MuxBundle<32, 4> aluBForwardMux;
	MuxBundle<32, 2> aluBInputMux;

	ALU<32> alu;
	FullAdderN<32> pcJumpAdder;
	BranchDetector branchDetector;
	MuxBundle<32, 2> aluOutOrPcIncMux;
	BufferEXMEM bufEXMEM;

	friend class CPU;
};

class CPU::Stage4 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect(const BufferEXMEM& stage3, const Wire& proceed);
	void Update();
	void PostUpdate();
	const BufferMEMWB& Out() const { return bufMEMWB; }
private:
	CPU::MainCache cache;
	SubWordSelector<32> byteSelect;
	SubWordSelector<32, 16> halfWordSelect;
	MuxBundle<32, 4> memOutWordMux;
	MuxBundle<32, 4> regWriteDataMux;
	BufferMEMWB bufMEMWB;

	friend class CPU;
};


void CPU::Stage1::Connect(const Bundle<32>& pcBranchAddr, const Wire& takeBranch, const Bundle<32>& pcJumpAddr, const Wire& takeJump, const Wire& proceed)
{
	// Program Counter. Select between PC + 4 and the calculated jump addr from the last executed branch
	pcBranchInMux.Connect({ pcIncrementer.Out(), pcBranchAddr }, takeBranch);
	pcJumpInMux.Connect({ pcBranchInMux.Out(), pcJumpAddr }, takeJump);

	pc.Connect(pcJumpInMux.Out(), proceed);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// Instruction memory
	instructionCache.Connect(pc.Out().Range<InsCache::ADDR_BITS>(0), InsCache::DataBundle(Wire::OFF), Wire::OFF, Wire::ON, Wire::OFF, Wire::OFF);

	// Out Buffer
	bufIFID.Connect(proceed, instructionCache.Out(), pcIncrementer.Out());
}

void CPU::Stage2::Connect(const BufferIFID& stage1, const BufferMEMWB& stage4, const Wire& proceed)
{
	// Opcode Decoding
	opcodeControl.Connect(stage1.IR.Opcode(), stage1.IR.Function());

	// Register File
	regFile.Connect(stage1.IR.RsAddr(), stage1.IR.RtAddr(),
		stage4.Rwrite.Out(), stage4.RWriteData.Out(),
		stage4.OpcodeControl().RegWrite());

	// Shift Amount
	reg2ShiftMux.Connect({ regFile.Out2(), stage1.IR.Shamt().ZeroExtend<32>() }, 
		opcodeControl.ShiftAmtOp());

	// Immediate Operand
	immExtender.Connect(stage1.IR.Immediate(), opcodeControl.OutBundle().MathOp());
	immShiftMux.Connect({ immExtender.Out(), stage1.IR.Immediate().ShiftZeroExtend<32>(16) }, 
		opcodeControl.OutBundle().LoadUpperImm());

	// Jump Address
	jumpAddrMux.Connect({ stage1.IR.Address().ZeroExtend<32>(), regFile.Out1() }, 
		opcodeControl.OutBundle().JumpReg());

	// Out Buffer
	bufIDEX.Connect(proceed, stage1.IR.RsAddr(), stage1.IR.RtAddr(), stage1.IR.RdAddr(),
		immShiftMux.Out(), regFile.Out1(), reg2ShiftMux.Out(), jumpAddrMux.Out(),
		stage1.PCinc.Out(), stage1.IR.Opcode(),
		opcodeControl.OutBundle(), opcodeControl.AluControl());
}

void CPU::Stage3::Connect(const BufferIDEX& stage2, const HazardUnit& hazard, const Wire& proceed)
{
	// Hardcoded Link in R31.
	regFileWriteAddrMux.Connect({ stage2.RT.Out(), stage2.RD.Out(), Bundle<5>(31U), Bundle<5>(31U) },
		{ &stage2.OpcodeControl().RFormat(), &stage2.OpcodeControl().JumpLink() });

	// ALU Input A
	aluAInputMux.Connect({ stage2.reg1.Out(), hazard.ForwardExMem(),
		hazard.ForwardMemWb(), hazard.ForwardMemWb() },
		hazard.AluRsMux());

	// ALU Input B
	aluBForwardMux.Connect({ stage2.reg2.Out(), hazard.ForwardExMem(), hazard.ForwardMemWb(), Bundle<32>::ERROR },
		hazard.AluRtMux());
	aluBInputMux.Connect({ aluBForwardMux.Out(), stage2.immExt.Out() }, stage2.OpcodeControl().AluBFromImm());
	
	// ALU
	alu.Connect(aluAInputMux.Out(), aluBInputMux.Out(), stage2.aluControl.Out());
	
	// PC Jump address calculation (A + B, no carry)
	pcJumpAdder.Connect(stage2.PCinc.Out(), stage2.immExt.Out(), Wire::OFF);
	
	// Branch Detection
	branchDetector.Connect(alu.Flags().Zero(), alu.Flags().Negative(),
		stage2.OpcodeControl().BranchSel(), stage2.OpcodeControl().Branch());

	// Bit of a hack? Drop the PC+4 into the alu out field for Jump Link instructions
	aluOutOrPcIncMux.Connect({ alu.Out(), stage2.PCinc.Out() }, stage2.OpcodeControl().JumpLink());
	
	// Out Buffer
	bufEXMEM.Connect(proceed, regFileWriteAddrMux.Out(), aluBForwardMux.Out(), aluOutOrPcIncMux.Out(),
		alu.Flags(), pcJumpAdder.Out(), branchDetector.Out(), stage2.OpcodeControl());
}

void CPU::Stage4::Connect(const BufferEXMEM& stage3, const Wire& proceed)
{
	// Main Memory
	const auto& memAddr = stage3.aluOut.Out();
	cache.Connect(memAddr.Range<MainCache::ADDR_BITS>(0), stage3.reg2.Out(),
		stage3.OpcodeControl().StoreOp(), stage3.OpcodeControl().LoadOp(),
		stage3.OpcodeControl().MemOpByte(), stage3.OpcodeControl().MemOpHalfWord());

	// Byte/Half/Word Selection
	byteSelect.Connect(cache.Out(), memAddr.Range<2>(0), stage3.OpcodeControl().LoadSigned());
	halfWordSelect.Connect(cache.Out(), Bundle<1>(memAddr[1]), stage3.OpcodeControl().LoadSigned());
	memOutWordMux.Connect({ cache.Out(), byteSelect.Out(), halfWordSelect.Out(), cache.Out() }, 
		{ &stage3.OpcodeControl().MemOpByte(),&stage3.OpcodeControl().MemOpHalfWord() });

	// SLT instruction
	Bundle<32> sltExtended = Bundle<32>::OFF;
	sltExtended.Connect(0, stage3.Flags().Negative());

	// Regfile Data Write
	regWriteDataMux.Connect({ memAddr, memOutWordMux.Out(), sltExtended, sltExtended },
		{ &stage3.OpcodeControl().LoadOp(), &stage3.OpcodeControl().SltOp() });
	
	// Out Buffer
	bufMEMWB.Connect(proceed, stage3.Rwrite.Out(), regWriteDataMux.Out(), stage3.OpcodeControl());
}


// ******** STAGE 1 INSTRUCTION FETCH ************
void CPU::Stage1::Update()
{
	pcBranchInMux.Update();
	pcJumpInMux.Update();
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
	reg2ShiftMux.Update();
	immExtender.Update();
	immShiftMux.Update();
	jumpAddrMux.Update();
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
	pcJumpAdder.Update();
	branchDetector.Update();
	aluOutOrPcIncMux.Update();
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
	halfWordSelect.Update();
	memOutWordMux.Update();
	regWriteDataMux.Update();
}
void CPU::Stage4::PostUpdate()
{
	bufMEMWB.Update();
}


CPU::CPU()
	: stage1(new Stage1(mMutex, mCV, stage1Ready))
	, stage2(new Stage2(mMutex, mCV, stage2Ready))
	, stage3(new Stage3(mMutex, mCV, stage3Ready))
	, stage4(new Stage4(mMutex, mCV, stage4Ready))
	, stage1Thread(&CPU::Stage1::ThreadedUpdate, stage1)
	, stage2Thread(&CPU::Stage2::ThreadedUpdate, stage2)
	, stage3Thread(&CPU::Stage3::ThreadedUpdate, stage3)
	, stage4Thread(&CPU::Stage4::ThreadedUpdate, stage4)
	, cycles(0)
{
	hazard.Connect(stage3->Out().Rwrite.Out(), stage3->Out().aluOut.Out(), stage3->Out().OpcodeControl().RegWrite(),
		stage4->Out().Rwrite.Out(), stage4->Out().RWriteData.Out(), stage4->Out().OpcodeControl().RegWrite(),
		stage2->Out().RS.Out(), stage2->Out().RT.Out());

	interlock.Connect(InstructionMem().NeedStall(), MainMem().NeedStall(), stage1->Out().IR.RsAddr(), stage1->Out().IR.RtAddr(),
		stage2->Out().RD.Out(), stage2->Out().OpcodeControl().LoadOp());

	stage1Thread.detach();
	stage2Thread.detach();
	stage3Thread.detach();
	stage4Thread.detach();
}

void CPU::Connect()
{
	stage1->Connect(stage3->Out().pcJumpAddr.Out(), stage3->Out().branchTaken.Out(), 
		stage2->Out().pcJumpAddr.Out(), stage2->Out().OpcodeControl().JumpOp(), interlock.FreezeOrBubbleInv());

	stage2->Connect(stage1->Out(), stage4->Out(), interlock.FreezeOrBubbleInv());
	stage3->Connect(stage2->Out(), hazard, interlock.FreezeInv());
	stage4->Connect(stage3->Out(), interlock.FreezeInv());
	cycles = 0;
}

void CPU::Update()
{
	{
		std::lock_guard<std::mutex> lk(mMutex);
		stage1Ready = stage2Ready = stage3Ready = stage4Ready = true;
	}
	mCV.notify_all();
	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCV.wait(lk, [this] {return !stage1Ready && !stage2Ready && !stage3Ready && !stage4Ready; });
	}
	interlock.Update();
	stage4->PostUpdate();
	stage3->PostUpdate();
	stage2->PostUpdate();
	stage1->PostUpdate();
	hazard.Update();
	cycles++;
}

const Bundle<32>& CPU::PC()
{
	return stage1->pc.Out();
}

CPU::InsCache& CPU::InstructionMem()
{
	return stage1->instructionCache;
}

CPU::MainCache& CPU::MainMem()
{
	return stage4->cache;
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
void CPU::PipelineStage::ThreadedUpdate()
{
	while (true)
	{
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mCV.wait(lk, [this] {return mReady; });
		}
		Update();
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mReady = false;
			lk.unlock();
			mCV.notify_all();
		}
	}
}
