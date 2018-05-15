#include "ProgramLoader.h"

ProgramLoader::ProgramLoader(CPU & cpu)
	: insMemory(cpu.instructionMem)
	, cur_addr(0)
{
	insMemory.Connect(addr_bundle.Range<0, CPU::InsMemory::ADDR_BITS>(), ins_bundle, Wire::ON);
}

void ProgramLoader::LoadInstruction(Instruction i)
{
	ins_bundle.Write(i.GetValue());
	addr_bundle.Write(cur_addr);
	insMemory.Update();
	cur_addr += 4;
}