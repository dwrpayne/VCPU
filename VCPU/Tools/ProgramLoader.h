#pragma once
#include <vector>
#include "Tools/MagicBundle.h"
#include "CPU/CPU.h"

class ProgramLoader
{
public:
	ProgramLoader(CPU& cpu);
	void Load(const std::vector<unsigned int>& instructions);

private:
	CPU::InsMemory& insMemory;
	MagicBundle<32> ins_bundle, addr_bundle;
	unsigned int cur_addr;
};

