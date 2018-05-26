#include <iostream>
#include <string>
#include "Debugger.h"
#include "ProgramLoader.h" 
#include "Assembler.h"


Debugger::Debugger(const std::string& source_filename)
{
	pCPU = new CPU();
	bPrintInstruction = false;
	bPrintRegisters = false;

	pAssembler = new Assembler(source_filename);


	ProgramLoader loader(*pCPU);
	loader.Load(pAssembler->GetBinary());
	pCPU->Connect();
	//pCPU->Update();
}

void Debugger::Start(int cycles)
{
	while (cycles > 0)
	{
		Step();

		if (pCPU->cycles % 1000 == 0)
		{
			long long ms = mCpuElapsedTime.count() / 1000;
			std::cout << pCPU->cycles << " cycles in " << ms << "ms. Average clock freq of " << (1000.0 * pCPU->cycles) / mCpuElapsedTime.count() << "kHz" << std::endl;
		}
		cycles--;
	}
}

void Debugger::Step()
{
	mLastInst.push_front(GetNextPCAddr() / 4);
	if (mLastInst.size() > 5)
	{
		mLastInst.pop_back();
	}

	if (bPrintInstruction)
	{
		PrintInstruction();
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	pCPU->Update();
	auto t2 = std::chrono::high_resolution_clock::now();


	mCpuElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	if (bPrintRegisters)
	{
		PrintRegisters();
	}
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->Registers().registers[reg].Out().Read();
}

int Debugger::GetMemoryVal(int addr)
{
	return pCPU->MainMem().mMemory.registers[addr / CPU::MainCache::WORD_BYTES / 8].Out().Read();
}

int Debugger::GetNextPCAddr()
{
	return pCPU->PC().Out().UnsignedRead();
}

void Debugger::PrintInstruction()
{
	static const char* stage[5] = { "IF", "ID", "EX", "MEM", "WB" };
	for (int i = 0; i < mLastInst.size(); ++i)
	{
		unsigned int addr = mLastInst[i];
		auto line = pAssembler->GetSourceLine(addr);
		std::cout << "\t0x" << std::hex << addr*4 << " " << stage[i] << std::dec << "\t" << line << std::endl;
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
