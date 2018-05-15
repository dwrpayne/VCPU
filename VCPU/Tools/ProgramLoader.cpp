#include "ProgramLoader.h"

ProgramLoader::ProgramLoader(CPU & cpu)
	: insMemory(cpu.instructionMem)
	, cur_addr(0)
{
	insMemory.Connect(addr_bundle.Range<0, CPU::InsMemory::ADDR_BITS>(), ins_bundle, Wire::ON);
}

void ProgramLoader::LoadInstruction(Opcode opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, Function func)
{
	unsigned int val = (opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
	WriteInstruction(val);
}

void ProgramLoader::LoadInstruction(Opcode opcode, unsigned int rs, unsigned int rt, unsigned short imm)
{
	unsigned int val = (opcode << 26) + (rs << 21) + (rt << 16) + imm;
	WriteInstruction(val);
}

inline void ProgramLoader::WriteInstruction(unsigned int val)
{
	ins_bundle.Write(val);
	addr_bundle.Write(cur_addr);
	insMemory.Update();
	cur_addr += 4;
}