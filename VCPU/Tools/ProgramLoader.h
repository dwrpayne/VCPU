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
	int availBytes;
	SystemBus& systembus;
	MagicBundle<CPU::WORD_SIZE> addrBundle;
	std::array<MagicBundle<CPU::WORD_SIZE>, CPU::CACHE_LINE_BITS/CPU::WORD_SIZE> wordBundles;
	Wire write;
	int curAddr;
};

