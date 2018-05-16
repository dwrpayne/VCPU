#include <iostream>

#include "Debugger.h"
#include "ProgramLoader.h"


Debugger::Debugger(std::vector<Instruction>& program)
	: mProgram(program)
	, pCPU(new CPU())
{
	pCPU = new CPU();
	bPrintInstruction = true;
	bStep = false;

	ProgramLoader loader(*pCPU);
	loader.Load(mProgram);
}

void Debugger::Start()
{
	pCPU->Connect();

	while (true)
	{
		if (bStep)
		{
			__debugbreak();
		}
		pCPU->Update();

		if (pCPU->cycles % 10 == 0)
		{
			PrintRegisters();
		}

		if (bPrintInstruction)
		{
			PrintInstruction();
		}
	}
}

void Debugger::PrintInstruction()
{
	unsigned int addr = pCPU->pc.Out().UnsignedRead() / 4;
	if (addr < mProgram.size())
	{
		std::cout << pCPU->cycles << "\t" << mProgram[pCPU->pc.Out().UnsignedRead() / 4] << std::endl;
	}
}



void Debugger::PrintRegisters()
{
	std::cout << "-------------REGISTER DUMP-------------" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << "$" << i + 8*j << " " << pCPU->regFile.registers[i + 8*j].Out().Read() << "\t\t";
		}
		std::cout << std::endl;
	}
}
