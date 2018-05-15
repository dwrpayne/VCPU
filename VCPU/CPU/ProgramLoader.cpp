#include "ProgramLoader.h"

ProgramLoader::ProgramLoader(CPU & c)
	: cpu(c)
	, cur_addr(0)
{
	cpu.ConnectToLoader(addr_bundle, ins_bundle);
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
	cpu.LoadInstruction();
	cur_addr += 4;
}