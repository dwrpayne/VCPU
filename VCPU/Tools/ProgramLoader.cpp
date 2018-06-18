#include "ProgramLoader.h"
#include "Program.h"

ProgramLoader::ProgramLoader(CPU & cpu)
	: insMemory(cpu.InstructionMem())
	, cur_addr(0)
{
	insMemory.Connect(addr_bundle.Range<CPU::InsCache::ADDR_BITS>(0), ins_bundle, Wire::OFF, Wire::ON, Wire::OFF, Wire::OFF);
}

void ProgramLoader::Load(const Program * program)
{
	assert(program->Instructions().size() < (insMemory.MEMORY_BYTES / 4));
	for (const auto& i : program->Instructions())
	{
		unsigned int bin = i.mBinary;
		ins_bundle.Write(bin);
		addr_bundle.Write(cur_addr);
		insMemory.UpdateUntilNoStall();
		cur_addr += 4;
	}
}
