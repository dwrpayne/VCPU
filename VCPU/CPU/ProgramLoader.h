#pragma once

#include "Instructions.h"
#include "MagicBundle.h"
#include "CPU.h"

class ProgramLoader
{
public:
	ProgramLoader(CPU& cpu);

	void LoadInstruction(Opcode opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, Function func);
	void LoadInstruction(Opcode opcode, unsigned int rs, unsigned int rt, unsigned short imm);

private:
	void WriteInstruction(unsigned int val);

	CPU::InsMemory& insMemory;
	MagicBundle<32> ins_bundle, addr_bundle;
	unsigned int cur_addr;
};

