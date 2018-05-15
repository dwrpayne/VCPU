#include <iostream>

#include "Debugger.h"
#include "ProgramLoader.h"
#include "CPU/Instructions.h"


Debugger::Debugger(CPU& cpu)
	: mCPU(cpu)
{
	bPrintInstruction = true;
}


Debugger::~Debugger()
{
}

void Debugger::LoadProgram()
{
	program.push_back({ ADDI, 0, 1, 1 });
	program.push_back({ ADDI, 0, 2, 1 });
	for (unsigned int i = 0; i < 25; i++)
	{
		program.push_back({ ADD, i + 1, i + 2, i + 3, 0 });
		program.push_back({ SW, 0, i + 3, i * 4 });
	}

	ProgramLoader loader(mCPU);
	loader.Load(program);
}

void Debugger::Start()
{
	mCPU.Connect();

	while (true)
	{
		mCPU.Update();

		if (mCPU.cycles % 10 == 0)
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
	unsigned int addr = mCPU.pc.Out().UnsignedRead() / 4;
	if (addr < program.size())
	{
		std::cout << mCPU.cycles << "\t" << program[mCPU.pc.Out().UnsignedRead() / 4] << std::endl;
	}
}

void Debugger::PrintRegisters()
{
	std::cout << "-------------REGISTER DUMP-------------" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << "R" << i + 8*j << " " << mCPU.regFile.registers[i + 8*j].Out().Read() << "\t\t";
		}
		std::cout << std::endl;
	}
}
