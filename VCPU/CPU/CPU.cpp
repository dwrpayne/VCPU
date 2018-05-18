#include "CPU.h"


void CPU::Connect()
{
	cycles = 0;
	// Internal Bundles must be created first
	Bundle<32> signExtImm(bufIFID.IR.Immediate()[15]);
	signExtImm.Connect(0, bufIFID.IR.Immediate());

	// ******** STAGE 1 BEGIN - INSTRUCTION FETCH ************

	// Program Counter
	pcInMux.Connect({ pcIncrementer.Out(), bufEXMEM.pcJumpAddr.Out() }, bufEXMEM.OpcodeControl().Branch());
	pc.Connect(pcInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// Instruction memory
	instructionMem.Connect(pc.Out().Range<0,InsMemory::ADDR_BITS>(), InsMemory::DataBundle(Wire::OFF), Wire::OFF);

	// ******** STAGE 1 END - INSTRUCTION FETCH ************

	bufIFID.Connect(instructionMem.Out(), pcIncrementer.Out());

	// ******** STAGE 2 BEGIN - INSTRUCTION DECODE ************
		
	// Opcode Decoding
	opcodeControl.Connect(bufIFID.IR.Opcode(), bufIFID.IR.Function());

	// Register File
	regFile.Connect(bufIFID.IR.RsAddr(), bufIFID.IR.RtAddr(), bufMEMWB.Rwrite.Out(), regWriteDataMux.Out(), bufMEMWB.OpcodeControl().RegWrite());
	
	// ******** STAGE 2 END - INSTRUCTION DECODE ************

	bufIDEX.Connect(bufIFID.IR.RtAddr(), bufIFID.IR.RdAddr(), signExtImm, regFile.Out1(), regFile.Out2(), bufIFID.PCinc.Out(), bufIFID.IR.Opcode(), opcodeControl.AsBundle(), opcodeControl.AluControl());

	// ******** STAGE 3 BEGIN - EXECUTION ************
	
	regFileWriteAddrMux.Connect({ bufIDEX.RT.Out(), bufIDEX.RD.Out() }, bufIDEX.OpcodeControl().RFormat());

	// ALU
	aluBInputMux.Connect({ bufIDEX.reg2.Out(), bufIDEX.signExt.Out() }, bufIDEX.OpcodeControl().AluBFromImm());
	alu.Connect(bufIDEX.reg1.Out(), aluBInputMux.Out(), bufIDEX.aluControl.Out());
	
	// PC Jump address calculation
	pcJumpAdder.Connect(bufIDEX.PCinc.Out(), bufIDEX.signExt.Out(), Wire::OFF);
	
	// ******** STAGE 3 END - EXECUTION ************

	bufEXMEM.Connect(regFileWriteAddrMux.Out(), bufIDEX.reg2.Out(), alu.Out(), pcJumpAdder.Out(), bufIDEX.OpcodeControl());

	// ******** STAGE 4 BEGIN - MEMORY STORE ************
	
	// Main Memory
	mainMem.Connect(bufEXMEM.aluOut.Out().Range<0,MainMemory::ADDR_BITS>(), bufEXMEM.reg2.Out(), bufEXMEM.OpcodeControl().StoreOp());

	// ******** STAGE 4 END - MEMORY STORE ************

	bufMEMWB.Connect(bufEXMEM.Rwrite.Out(), bufEXMEM.aluOut.Out(), mainMem.Out(), bufEXMEM.OpcodeControl());

	// ******** STAGE 5 BEGIN - WRITEBACK ************
	Bundle<32> sltExtended(Wire::OFF);
	sltExtended.Connect(0, alu.Negative());
	regWriteDataMux.Connect({ bufMEMWB.aluOut.Out(), bufMEMWB.memOut.Out(), sltExtended, sltExtended }, 
		{ &bufMEMWB.OpcodeControl().LoadOp(), &bufMEMWB.OpcodeControl().SltInst() });
}

void CPU::Update1()
{
	// ******** STAGE 1 INSTRUCTION FETCH ************
	pcInMux.Update();
	pc.Update();
	pcIncrementer.Update();
	instructionMem.Update();
	bufIFID.Update();
}

void CPU::Update2()
{
	// ******** STAGE 2 INSTRUCTION DECODE ************
	opcodeControl.Update();
	regFile.Update();
	bufIDEX.Update();
}

void CPU::Update3()
{
	// ******** STAGE 3 EXECUTION ************
	regFileWriteAddrMux.Update();
	aluBInputMux.Update();
	alu.Update();
	pcJumpAdder.Update();
	bufEXMEM.Update();
}

void CPU::Update4()
{
	// ******** STAGE 4 BEGIN - MEMORY STORE ************
	mainMem.Update();
	bufMEMWB.Update();
}

void CPU::Update5()
{
	// ******** STAGE 5 BEGIN - WRITEBACK ************
	regWriteDataMux.Update();
	cycles++;
}

void CPU::Update()
{
	Update1();
	Update2();
	Update3();
	Update4();
	Update5();
}
