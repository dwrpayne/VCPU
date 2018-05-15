#include "CPU.h"


void CPU::Connect()
{
	cycles = 0;
	// Internal Bundles must be created first
	Bundle<32> signExtImm(ir.Immediate()[15]);
	signExtImm.Connect(0, ir.Immediate());

	// Program Counter
	pc.Connect(pcInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// PC Jump address calculation
	Bundle<32> addrOffset(Wire::OFF);
	addrOffset.Connect(0, signExtImm);
	pcJumpAdder.Connect(pcIncrementer.Out(), addrOffset, Wire::OFF);

	pcInMux.Connect({ pcIncrementer.Out(), pcJumpAdder.Out() }, opcodeControl.Branch());

	// Instruction memory
	instructionMem.Connect(pc.Out().Range<0,InsMemory::ADDR_BITS>(), InsMemory::DataBundle(Wire::OFF), Wire::OFF);
	ir.Connect(instructionMem.Out(), Wire::ON);
	
	// Opcode Decoding
	opcodeControl.Connect(ir.Opcode());
	aluControl.Connect(opcodeControl.LoadStore(), opcodeControl.Branch(), opcodeControl.IFormat(), opcodeControl.RFormat(), ir.Opcode(), ir.Function());
	
	// Register File
	regFileWriteAddrMux.Connect({ ir.RtAddr(), ir.RdAddr() }, opcodeControl.RFormat());
	regWriteDataMux.Connect({ aluOut.Out(), mainMem.Out() }, opcodeControl.LoadOp());
	regFile.Connect(ir.RsAddr(), ir.RtAddr(), regFileWriteAddrMux.Out(), regWriteDataMux.Out(), opcodeControl.RegWrite());

	// ALU
	aluBInputMux.Connect({ regFile.Out2(), signExtImm }, opcodeControl.AluBFromImm());
	alu.Connect(regFile.Out1(), aluBInputMux.Out(), aluControl.AluControl());
	aluOut.Connect(alu.Out(), Wire::ON);

	// Main Memory
	mainMem.Connect(aluOut.Out().Range<0,MainMemory::ADDR_BITS>(), regFile.Out2(), opcodeControl.StoreOp());
}

void CPU::Update()
{
	pcInMux.Update();
	pc.Update();
	pcIncrementer.Update();
	instructionMem.Update();
	ir.Update();
	opcodeControl.Update();
	aluControl.Update();
	regFile.Update();
	aluBInputMux.Update();
	alu.Update();
	aluOut.Update();
	mainMem.Update();
	regFileWriteAddrMux.Update();
	regWriteDataMux.Update();
	cycles++;
}

void CPU::ConnectToLoader(Bundle<32>& addr, Bundle<32> ins)
{
	instructionMem.Connect(addr.Range<0, InsMemory::ADDR_BITS>(), ins, Wire::ON);
}

void CPU::LoadInstruction()
{
	instructionMem.Update();
}
