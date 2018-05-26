#include "CPU.h"
#include <vector>
#include <future>


class CPU::Stage1 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect();
	void Update();
	void Update2();
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
	void Connect();
	void Update();
	void Update2();
	const BufferIDEX& Out() const { return bufIDEX; }
private:
	OpcodeDecoder opcodeControl;
	CPU::RegFile regFile;
	BufferIDEX bufIDEX;

	friend class CPU;
};

class CPU::Stage3 : public PipelineStage
{
public:
	using PipelineStage::PipelineStage;
	void Connect();
	void Update();
	void Update2();
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
	void Connect();
	void Update();
	void Update2();
	const BufferMEMWB& Out() const { return bufMEMWB; }
private:
	CPU::MainCache cache;
	MuxBundle<32, 4> regWriteDataMux;
	BufferMEMWB bufMEMWB;

	friend class CPU;
};


void CPU::Stage1::Connect()
{
	// Program Counter
	pcInMux.Connect({ pcIncrementer.Out(), cpu.stage3->Out().pcJumpAddr.Out() }, cpu.stage3->BranchTaken());
	pc.Connect(pcInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// Instruction memory
	instructionCache.Connect(pc.Out().Range<InsCache::ADDR_BITS>(0), InsCache::DataBundle(Wire::OFF), Wire::OFF);
	
	bufIFID.Connect(Wire::ON, instructionCache.Out(), pcIncrementer.Out());
}

void CPU::Stage2::Connect()
{
	// Opcode Decoding
	opcodeControl.Connect(cpu.stage1->Out().IR.Opcode(), cpu.stage1->Out().IR.Function());

	// Register File
	regFile.Connect(cpu.stage1->Out().IR.RsAddr(), cpu.stage1->Out().IR.RtAddr(),
		cpu.stage4->Out().Rwrite.Out(), cpu.stage4->Out().RWriteData.Out(),
		cpu.stage4->Out().OpcodeControl().RegWrite());

	Bundle<32> signExtImm(cpu.stage1->Out().IR.Immediate()[15]);
	signExtImm.Connect(0, cpu.stage1->Out().IR.Immediate());

	bufIDEX.Connect(Wire::ON, cpu.stage1->Out().IR.RsAddr(), cpu.stage1->Out().IR.RtAddr(),
		cpu.stage1->Out().IR.RdAddr(),
		signExtImm, regFile.Out1(), regFile.Out2(),
		cpu.stage1->Out().PCinc.Out(), cpu.stage1->Out().IR.Opcode(),
		opcodeControl.OutBundle(), opcodeControl.AluControl());
}

void CPU::Stage3::Connect()
{
	regFileWriteAddrMux.Connect({ cpu.stage2->Out().RT.Out(), cpu.stage2->Out().RD.Out() }, 
		cpu.stage2->Out().OpcodeControl().RFormat());

	// ALU
	aluAInputMux.Connect({ cpu.stage2->Out().reg1.Out(), cpu.hazard.ForwardExMem(),
		cpu.hazard.ForwardMemWb(), cpu.hazard.ForwardMemWb() },
		cpu.hazard.AluRsMux());

	aluBImmediateOr.Connect(cpu.hazard.AluRtMux(), Bundle<2>(cpu.stage2->Out().OpcodeControl().AluBFromImm()));
	aluBInputMux.Connect({ cpu.stage2->Out().reg2.Out(), cpu.hazard.ForwardExMem(), 
							cpu.hazard.ForwardMemWb(), cpu.stage2->Out().signExt.Out() }, 
		aluBImmediateOr.Out());

	alu.Connect(aluAInputMux.Out(), aluBInputMux.Out(), cpu.stage2->Out().aluControl.Out());
	
	// PC Jump address calculation (A + B, no carry)
	pcJumpAdder.Connect(cpu.stage2->Out().PCinc.Out(), cpu.stage2->Out().signExt.Out(), Wire::OFF);
	
	branchDetector.Connect(alu.Flags().Zero(), alu.Flags().Negative(),
		cpu.stage2->Out().OpcodeControl().BranchSel(), cpu.stage2->Out().OpcodeControl().Branch());
	
	bufEXMEM.Connect(Wire::ON, regFileWriteAddrMux.Out(), cpu.stage2->Out().reg2.Out(), 
		alu.Out(), alu.Flags(), pcJumpAdder.Out(), cpu.stage2->Out().OpcodeControl());
}

void CPU::Stage4::Connect()
{

	// Main Memory
	cache.Connect(cpu.stage3->Out().aluOut.Out().Range<MainCache::ADDR_BITS>(0), cpu.stage3->Out().reg2.Out(), 
		cpu.stage3->Out().OpcodeControl().StoreOp());

	Bundle<32> sltExtended(Wire::OFF);
	sltExtended.Connect(0, cpu.stage3->Out().Flags().Negative());

	regWriteDataMux.Connect({ cpu.stage3->Out().aluOut.Out(), cache.Out(), sltExtended, sltExtended },
		{ &cpu.stage3->Out().OpcodeControl().LoadOp(), &cpu.stage3->Out().OpcodeControl().SltOp() });

	bufMEMWB.Connect(Wire::ON, cpu.stage3->Out().Rwrite.Out(), regWriteDataMux.Out(), cpu.stage3->Out().OpcodeControl());
}

void CPU::Stage1::Update()
{
	// ******** STAGE 1 INSTRUCTION FETCH ************
	pcInMux.Update();
	pc.Update();
	pcIncrementer.Update();
	instructionCache.Update();
}
void CPU::Stage1::Update2()
{
	bufIFID.Update();
}

void CPU::Stage2::Update()
{
	// ******** STAGE 2 INSTRUCTION DECODE ************
	opcodeControl.Update();
	regFile.Update();
}
void CPU::Stage2::Update2()
{
	bufIDEX.Update();
}

void CPU::Stage3::Update()
{
	// ******** STAGE 3 EXECUTION ************
	regFileWriteAddrMux.Update();
	aluAInputMux.Update();
	aluBImmediateOr.Update();
	aluBInputMux.Update();
	alu.Update();
	pcJumpAdder.Update();
	branchDetector.Update();
}
void CPU::Stage3::Update2()
{
	bufEXMEM.Update();
}

void CPU::Stage4::Update()
{
	// ******** STAGE 4 BEGIN - MEMORY STORE ************
	cache.Update();
	regWriteDataMux.Update();
}
void CPU::Stage4::Update2()
{
	bufMEMWB.Update();
}


CPU::CPU()
	: stage1(new Stage1(*this))
	, stage2(new Stage2(*this))
	, stage3(new Stage3(*this))
	, stage4(new Stage4(*this))
{
	hazard.Connect(stage3->Out().Rwrite.Out(), stage3->Out().aluOut.Out(), stage3->Out().OpcodeControl().RegWrite(),
		stage4->Out().Rwrite.Out(), stage4->Out().RWriteData.Out(), stage4->Out().OpcodeControl().RegWrite(),
		stage2->Out().RS.Out(), stage2->Out().RT.Out());
}

void CPU::Connect()
{
	stage1->Connect();
	stage2->Connect();
	stage3->Connect();
	stage4->Connect();
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
	stage4->Update2();
	stage3->Update2();
	stage2->Update2();
	stage1->Update2();
	hazard.Update();
	cycles++;
}

Register<32> CPU::PC()
{
	return stage1->pc;
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
