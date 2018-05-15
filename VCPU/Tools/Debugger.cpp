#include <iostream>

#include "Debugger.h"
#include "ProgramLoader.h"
#include "CPU/Instructions.h"


Debugger::Debugger(CPU& cpu, std::vector<Instruction>& program)
	: mCPU(cpu)
	, mProgram(program)
{
	bPrintInstruction = true;
	bStep = false;

	ProgramLoader loader(mCPU);
	loader.Load(mProgram);
}

void Debugger::Start()
{
	mCPU.Connect();

	while (true)
	{
		if (bStep)
		{
			__debugbreak();
		}
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
	if (addr < mProgram.size())
	{
		std::cout << mCPU.cycles << "\t" << mProgram[mCPU.pc.Out().UnsignedRead() / 4] << std::endl;
	}
}

void Debugger::PrintRegisters()
{
	std::cout << "-------------REGISTER DUMP-------------" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << "$" << i + 8*j << " " << mCPU.regFile.registers[i + 8*j].Out().Read() << "\t\t";
		}
		std::cout << std::endl;
	}
}
