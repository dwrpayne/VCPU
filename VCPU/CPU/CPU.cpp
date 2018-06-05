#include "CPU.h"
#include <vector>
#include <future>


class CPU::Stage1 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect(const Bundle<32>& pcJumpAddr, const Wire& takeBranch, const Wire& proceed);
	void Update();
	void PostUpdate();
	const BufferIFID& Out() const { return bufIFID; }
private:
	MuxBundle<32, 2> pcInMux;
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
	const Wire& BranchTaken() const { return branchDetector.Out(); }
private:
	MuxBundle<CPU::RegFile::ADDR_BITS, 2> regFileWriteAddrMux;

	MuxBundle<32, 4> aluAInputMux;
	
	MultiGate<OrGate, 2> aluBImmediateOr;
	MuxBundle<32, 4> aluBInputMux;

	ALU<32> alu;
	FullAdderN<32> pcJumpAdder;
	BranchDetector branchDetector;
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
	MuxBundle<32, 4> regWriteDataMux;
	BufferMEMWB bufMEMWB;

	friend class CPU;
};


void CPU::Stage1::Connect(const Bundle<32>& pcJumpAddr, const Wire& takeBranch, const Wire& proceed)
{
	// Program Counter
	pcInMux.Connect({ pcIncrementer.Out(), pcJumpAddr }, takeBranch);
	pc.Connect(pcInMux.Out(), proceed);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// Instruction memory
	instructionCache.Connect(pc.Out().Range<InsCache::ADDR_BITS>(0), InsCache::DataBundle(Wire::OFF), Wire::OFF, Wire::ON);
	
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

	Bundle<32> shiftAmtExt(Wire::OFF);
	shiftAmtExt.Connect(0, stage1.IR.Shamt());

	reg2ShiftMux.Connect({ regFile.Out2(), shiftAmtExt }, opcodeControl.ShiftAmtOp());

	Bundle<32> signExtImm(stage1.IR.Immediate()[15]);
	signExtImm.Connect(0, stage1.IR.Immediate());

	bufIDEX.Connect(proceed, stage1.IR.RsAddr(), stage1.IR.RtAddr(), stage1.IR.RdAddr(),
		signExtImm, regFile.Out1(), reg2ShiftMux.Out(),
		stage1.PCinc.Out(), stage1.IR.Opcode(),
		opcodeControl.OutBundle(), opcodeControl.AluControl());
}

void CPU::Stage3::Connect(const BufferIDEX& stage2, const HazardUnit& hazard, const Wire& proceed)
{
	regFileWriteAddrMux.Connect({ stage2.RT.Out(), stage2.RD.Out() }, 
		stage2.OpcodeControl().RFormat());

	// ALU
	aluAInputMux.Connect({ stage2.reg1.Out(), hazard.ForwardExMem(),
		hazard.ForwardMemWb(), hazard.ForwardMemWb() },
		hazard.AluRsMux());

	aluBImmediateOr.Connect(hazard.AluRtMux(), Bundle<2>(stage2.OpcodeControl().AluBFromImm()));
	aluBInputMux.Connect({ stage2.reg2.Out(), hazard.ForwardExMem(), 
							hazard.ForwardMemWb(), stage2.signExt.Out() }, 
		aluBImmediateOr.Out());

	alu.Connect(aluAInputMux.Out(), aluBInputMux.Out(), stage2.aluControl.Out());
	
	// PC Jump address calculation (A + B, no carry)
	pcJumpAdder.Connect(stage2.PCinc.Out(), stage2.signExt.Out(), Wire::OFF);
	
	branchDetector.Connect(alu.Flags().Zero(), alu.Flags().Negative(),
		stage2.OpcodeControl().BranchSel(), stage2.OpcodeControl().Branch());
	
	bufEXMEM.Connect(proceed, regFileWriteAddrMux.Out(), stage2.reg2.Out(),
		alu.Out(), alu.Flags(), pcJumpAdder.Out(), stage2.OpcodeControl());
}

void CPU::Stage4::Connect(const BufferEXMEM& stage3, const Wire& proceed)
{
	// Main Memory
	cache.Connect(stage3.aluOut.Out().Range<MainCache::ADDR_BITS>(0), stage3.reg2.Out(), 
		stage3.OpcodeControl().StoreOp(), stage3.OpcodeControl().LoadOp());

	Bundle<32> sltExtended(Wire::OFF);
	sltExtended.Connect(0, stage3.Flags().Negative());

	regWriteDataMux.Connect({ stage3.aluOut.Out(), cache.Out(), sltExtended, sltExtended },
		{ &stage3.OpcodeControl().LoadOp(), &stage3.OpcodeControl().SltOp() });

	bufMEMWB.Connect(proceed, stage3.Rwrite.Out(), regWriteDataMux.Out(), stage3.OpcodeControl());
}


// ******** STAGE 1 INSTRUCTION FETCH ************
void CPU::Stage1::Update()
{
	pcInMux.Update();
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
	aluBImmediateOr.Update();
	aluBInputMux.Update();
	alu.Update();
	pcJumpAdder.Update();
	branchDetector.Update();
}
void CPU::Stage3::PostUpdate()
{
	bufEXMEM.Update();
}


// ******** STAGE 4 BEGIN - MEMORY STORE ************
void CPU::Stage4::Update()
{
	cache.Update();
	regWriteDataMux.Update();
}
void CPU::Stage4::PostUpdate()
{
	bufMEMWB.Update();
}


CPU::CPU()
	: stage1(new Stage1())
	, stage2(new Stage2())
	, stage3(new Stage3())
	, stage4(new Stage4())
{
	hazard.Connect(stage3->Out().Rwrite.Out(), stage3->Out().aluOut.Out(), stage3->Out().OpcodeControl().RegWrite(),
		stage4->Out().Rwrite.Out(), stage4->Out().RWriteData.Out(), stage4->Out().OpcodeControl().RegWrite(),
		stage2->Out().RS.Out(), stage2->Out().RT.Out());

	interlock.Connect(InstructionMem().NeedStall(), MainMem().NeedStall(), stage1->Out().IR.RsAddr(), stage1->Out().IR.RtAddr(),
		stage2->Out().RD.Out(), stage2->Out().OpcodeControl().LoadOp());
}

void CPU::Connect()
{
	stage1->Connect(stage3->Out().pcJumpAddr.Out(), stage3->BranchTaken(), interlock.FreezeOrBubbleInv());
	stage2->Connect(stage1->Out(), stage4->Out(), interlock.FreezeOrBubbleInv());
	stage3->Connect(stage2->Out(), hazard, interlock.FreezeInv());
	stage4->Connect(stage3->Out(), interlock.FreezeInv());
	cycles = 0;
}

void CPU::Update()
{
	std::vector<std::future<void>> futures;
	futures.push_back(std::async([this]() {stage1->Update(); }));
	futures.push_back(std::async([this]() {stage2->Update(); }));
	futures.push_back(std::async([this]() {stage3->Update(); }));
	futures.push_back(std::async([this]() {stage4->Update(); }));
	for (auto& f : futures)
	{
		f.get();
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

