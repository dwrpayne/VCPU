#include "ProgramLoader.h"

ProgramLoader::ProgramLoader(CPU & cpu)
	: insMemory(cpu.InstructionMem())
	, cur_addr(0)
{
	insMemory.Connect(addr_bundle.Range<CPU::InsMemory::ADDR_BITS>(0), ins_bundle, Wire::ON);
}

void ProgramLoader::Load(const std::vector<unsigned int>& instructions)
{
	for (const auto& i : instructions)
	{
		ins_bundle.Write(i);
		addr_bundle.Write(cur_addr);
		insMemory.Update();
		cur_addr += 4;
	}
}