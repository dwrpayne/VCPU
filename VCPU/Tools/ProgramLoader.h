#pragma once
#include <vector>
#include "Instruction.h"
#include "CPU/MagicBundle.h"
#include "CPU/CPU.h"

class ProgramLoader
{
public:
	ProgramLoader(CPU& cpu);
	void Load(const std::vector<Instruction>& instructions);

private:
	CPU::InsMemory& insMemory;
	MagicBundle<32> ins_bundle, addr_bundle;
	unsigned int cur_addr;
};

