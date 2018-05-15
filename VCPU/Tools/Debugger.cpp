#include <iostream>

#include "Debugger.h"
#include "ProgramLoader.h"
#include "CPU/Instructions.h"


Debugger::Debugger(CPU& cpu)
	: mCPU(cpu)
{
}


Debugger::~Debugger()
{
}

void Debugger::LoadProgram()
{
	ProgramLoader loader(mCPU);
	loader.LoadInstruction({ ADDI, 0, 1, 1 });
	loader.LoadInstruction({ ADDI, 0, 2, 1 });
	for (unsigned int i = 0; i < 25; i++)
	{
		loader.LoadInstruction({ ADD, i + 1, i + 2, i + 3, 0});
		loader.LoadInstruction({ SW, 0, i + 3, i * 4 });
	}
}
void Debugger::Start()
{
	mCPU.Connect();

	while (true)
	{
		mCPU.Update();

		if (mCPU.cycles % 1000 == 0)
		{
			std::cout << mCPU.cycles << std::endl;
		}

		if (bPrintInstruction)
		{
			PrintInstruction();
		}
	}
}

void Debugger::PrintInstruction()
{
	//std::cout << mCPU.bufIFID.IR.Opcode().Read() << 
}
