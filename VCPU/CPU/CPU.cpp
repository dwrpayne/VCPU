#include "CPU.h"


void CPU::Connect()
{
	// Internal Bundles must be created first
	Bundle<32> signExtImm(ir.Immediate()[15]);
	signExtImm.Connect(0, ir.Immediate());

	// Dummy wires that aren't hooked up yet
	const Wire& xxxpcSrc = Wire::OFF;
	const Bundle<32> xxxregWriteData = aluOut.Out();
	const Wire& xxxregWrite = Wire::OFF;
	const Wire& xxxaluBInputSel = Wire::OFF;
	const Bundle<4> xxxaluOpcode = ir.Function().Range<0, 4>();

	// Program Counter
	pc.Connect(pcInMux.Out(), Wire::ON);

	// PC Increment. Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	// PC Jump address calculation
	Bundle<32> addrOffset(Wire::OFF);
	addrOffset.Connect(2, signExtImm.Range<0, 30>());
	pcJumpAdder.Connect(pcIncrementer.Out(), addrOffset, Wire::OFF);

	pcInMux.Connect({ pcIncrementer.Out(), pcJumpAdder.Out() }, xxxpcSrc);

	// Instruction memory
	// Drop the low 2 bits of the PC, grab the next chunk.
	instructionMem.Connect(pc.Out().Range<2, InsMemory::ADDR_BITS+2>(), InsMemory::DataBundle(Wire::OFF), Wire::OFF);
	ir.Connect(instructionMem.Out(), Wire::ON);

	// Register File
	regFile.Connect(ir.RsAddr(), ir.RtAddr(), ir.RdAddr(), xxxregWriteData, xxxregWrite);

	// ALU
	aluBInputMux.Connect({ regFile.Out2(), signExtImm }, xxxaluBInputSel);
	alu.Connect(regFile.Out1(), aluBInputMux.Out(), xxxaluOpcode);
	aluOut.Connect(alu.Out(), Wire::ON);
}

void CPU::Update()
{
	pcInMux.Update();
	pc.Update();
	pcIncrementer.Update();
	instructionMem.Update();
	ir.Update();
	regFile.Update();
	aluBInputMux.Update();
	alu.Update();
	aluOut.Update();
}

void CPU::ConnectToLoader(Bundle<32>& addr, Bundle<32> ins)
{
	instructionMem.Connect(addr.Range<2, InsMemory::ADDR_BITS + 2>(), ins, Wire::ON);
}

void CPU::LoadInstruction()
{
	instructionMem.Update();
}
