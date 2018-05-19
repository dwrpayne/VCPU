#include <iostream>

#include "Debugger.h"
#include "ProgramLoader.h"


Debugger::Debugger(std::vector<Instruction>& program)
	: mProgram(program)
{
	pCPU = new CPU();
	bPrintInstruction = true;
	bPrintRegisters = true;

	ProgramLoader loader(*pCPU);
	loader.Load(mProgram);
	pCPU->Connect();
	pCPU->Update();
}

void Debugger::Start()
{
	while (true)
	{
		pCPU->Update();

		if (pCPU->cycles % 100 == 0)
		{
			PrintRegisters();
		}

		if (bPrintInstruction)
		{
			PrintInstruction();
		}
	}
}

void Debugger::Step()
{
	PrintInstruction();
	pCPU->Update();
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->regFile.registers[reg].Out().Read();
}

int Debugger::GetMemoryVal(int addr)
{
	return pCPU->mainMem.registers[addr / CPU::MainMemory::WORD_LEN].Out().Read();
}

int Debugger::GetNextPCAddr()
{
	return pCPU->pc.Out().Read();
}

void Debugger::PrintInstruction()
{
	unsigned int addr = pCPU->pc.Out().UnsignedRead() / 4;
	if (addr < mProgram.size())
	{
		std::cout << pCPU->cycles << "\t0x" << std::hex << GetNextPCAddr() << std::dec << "\t" << mProgram[pCPU->pc.Out().UnsignedRead() / 4] << std::endl;
	}
}


void Debugger::PrintRegisters()
{
	std::cout << "-------------REGISTER DUMP-------------" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << "$" << i + 8*j << " " << GetRegisterVal(i+8*j) << "\t\t";
		}
		std::cout << std::endl;
	}
}
