#include "CPU.h"

void CPU::Connect()
{
	pc.Connect(pcInMux.Out(), Wire::ON);

	// Instruction width is 4, hardwired.
	Bundle<32> insWidth(Wire::OFF);
	insWidth.Connect(2, Wire::ON);
	pcIncrementer.Connect(pc.Out(), insWidth, Wire::OFF);

	const Wire& XpcSrc = Wire::OFF;
	pcInMux.Connect({ pcIncrementer.Out(), pcJumpAdder.Out() }, XpcSrc);

	// Drop the low 2 bits of the PC, grab the next chunk.
	instructionMem.Connect(pc.Out().Range<2, InsMemory::ADDR_BITS+2>(), InsMemory::DataBundle(Wire::OFF), Wire::OFF);

	
}
