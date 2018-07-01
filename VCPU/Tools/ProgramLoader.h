#pragma once
#include <vector>
#include "Tools/MagicBundle.h"
#include "CPU/CPU.h"

class ProgramLoader
{
public:
	ProgramLoader(CPU& cpu);
	~ProgramLoader();
	void Load(const class Program* program);

private:
	void LoadInstructions(const class Program* program);
	void LoadText(const class Program* program);
	
	SystemBus& systembus;
	CPU::InsMemory& memory;
	MagicBundle<CPU::WORD_SIZE> addrBundle;
	Bundle<CPU::CACHE_LINE_BITS> dataBundle;
	std::array<MagicBundle<CPU::WORD_SIZE>, CPU::CACHE_LINE_BITS/CPU::WORD_SIZE> wordBundles;
	Wire write;
	Wire req;
	int curAddr;
};

