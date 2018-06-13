#pragma once
#include <vector>
#include "Tools/MagicBundle.h"
#include "CPU/CPU.h"

class ProgramLoader
{
public:
	ProgramLoader(CPU& cpu);
	void Load(const class Program* program);

private:
	CPU::InsCache& insMemory;
	MagicBundle<CPU::WORD_SIZE> ins_bundle, addr_bundle;
	unsigned int cur_addr;
};

