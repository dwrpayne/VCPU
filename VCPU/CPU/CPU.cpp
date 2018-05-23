#include "CPU.h"


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
	CPU::InsMemory instructionMem;
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
private:
	MuxBundle<CPU::RegFile::ADDR_BITS, 2> regFileWriteAddrMux;
	MuxBundle<32, 2> aluBInputMux;
	ALU<32> alu;
	FullAdderN<32> pcJumpAdder;
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
	const Wire& BranchTaken() const { return branchDetector.Out(); }
private:
	BranchDetector branchDetector;
	CPU::MainCache cache;
	CPU::MainMemory mainMem;
	MuxBundle<32, 4> regWriteDataMux;
	BufferMEMWB bufMEMWB;

	friend class CPU;
};


void CPU::Stage1::Connect()
{
	// Program Counter
	pcInMux.Connect({ pcIncrementer.Out(), cpu.stage3->Out().pcJumpAddr.Out() }, cpu.stage4->BranchTaken());
	pc.Connect(pcInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// Instruction memory
	instructionCache.Connect(pc.Out().Range<InsMemory::ADDR_BITS>(0), InsMemory::DataBundle(Wire::OFF), Wire::OFF, instructionMem.OutLine());
	instructionMem.Connect(pc.Out().Range<InsMemory::ADDR_BITS>(0), InsMemory::DataBundle(Wire::OFF), Wire::OFF);
	
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

	bufIDEX.Connect(Wire::ON, cpu.stage1->Out().IR.RtAddr(), cpu.stage1->Out().IR.RdAddr(),
		signExtImm, regFile.Out1(), regFile.Out2(),
		cpu.stage1->Out().PCinc.Out(), cpu.stage1->Out().IR.Opcode(),
		opcodeControl.OutBundle(), opcodeControl.AluControl());
}

void CPU::Stage3::Connect()
{
	regFileWriteAddrMux.Connect({ cpu.stage2->Out().RT.Out(), cpu.stage2->Out().RD.Out() }, 
		cpu.stage2->Out().OpcodeControl().RFormat());

	// ALU
	aluBInputMux.Connect({ cpu.stage2->Out().reg2.Out(), cpu.stage2->Out().signExt.Out() }, cpu.stage2->Out().OpcodeControl().AluBFromImm());
	alu.Connect(cpu.stage2->Out().reg1.Out(), aluBInputMux.Out(), cpu.stage2->Out().aluControl.Out());
	
	// PC Jump address calculation (A + B, no carry)
	pcJumpAdder.Connect(cpu.stage2->Out().PCinc.Out(), cpu.stage2->Out().signExt.Out(), Wire::OFF);
	
	bufEXMEM.Connect(Wire::ON, regFileWriteAddrMux.Out(), cpu.stage2->Out().reg2.Out(), 
		alu.Out(), alu.Flags(), pcJumpAdder.Out(), cpu.stage2->Out().OpcodeControl());
}

void CPU::Stage4::Connect()
{
	branchDetector.Connect(cpu.stage3->Out().Flags().Zero(), cpu.stage3->Out().Flags().Negative(), 
		cpu.stage3->Out().OpcodeControl().BranchSel(), cpu.stage3->Out().OpcodeControl().Branch());

	// Main Memory
	cache.Connect(cpu.stage3->Out().aluOut.Out().Range<MainMemory::ADDR_BITS>(0), cpu.stage3->Out().reg2.Out(), 
		cpu.stage3->Out().OpcodeControl().StoreOp(), mainMem.OutLine());

	mainMem.Connect(cpu.stage3->Out().aluOut.Out().Range<MainMemory::ADDR_BITS>(0), cpu.stage3->Out().reg2.Out(), 
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
	instructionMem.Update();
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
	aluBInputMux.Update();
	alu.Update();
	pcJumpAdder.Update();
}
void CPU::Stage3::Update2()
{
	bufEXMEM.Update();
}

void CPU::Stage4::Update()
{
	// ******** STAGE 4 BEGIN - MEMORY STORE ************
	branchDetector.Update();
	cache.Update();
	mainMem.Update();
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
	stage1->Update();
	stage1->Update2();
	stage2->Update();
	stage2->Update2();
	stage3->Update();
	stage3->Update2();
	stage4->Update();
	stage4->Update2();
	cycles++;
}

Register<32> CPU::PC()
{
	return stage1->pc;
}

CPU::InsMemory& CPU::InstructionMem()
{
	return stage1->instructionMem;
}

CPU::MainMemory& CPU::MainMem()
{
	return stage4->mainMem;
}

CPU::RegFile& CPU::Registers()
{
	return stage2->regFile;
}
