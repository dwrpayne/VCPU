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
	bPrintOutputReg = true;
	bPrintDataForward = true;

	pAssembler = new Assembler(source_filename);


	ProgramLoader loader(*pCPU);
	loader.Load(pAssembler->GetBinary());
	pCPU->Connect();
}

void Debugger::Start(int cycles)
{
	while (cycles != 0)
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
	auto t1 = std::chrono::high_resolution_clock::now();
	pCPU->Update();
	auto t2 = std::chrono::high_resolution_clock::now();
	
	mCpuElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	mLastInstructions.push_front(GetNextPCAddr() / 4);
	if (mLastInstructions.size() > 5)
	{
		mLastInstructions.pop_back();
	}

	if (bPrintInstruction)
	{
		PrintInstruction();
	}
	
	if (bPrintRegisters)
	{
		PrintRegisters();
	}

	if (bPrintOutputReg)
	{
		PrintOutputReg();
	}

	if (bPrintDataForward)
	{
		PrintDataForward();
	}
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->Registers().registers[reg].Out().Read();
}

int Debugger::GetMemoryVal(int addr)
{
	return pCPU->MainMem().mMemory.registers[addr / CPU::MainCache::WORD_BYTES].Out().Read();
}

int Debugger::GetNextPCAddr()
{
	return pCPU->PC().Out().UnsignedRead();
}

void Debugger::PrintInstruction()
{
	std::cout << "--------- CYCLE " << pCPU->cycles << " ---------------" << std::endl;
	static const char* STAGE[5] = { "IF", "ID", "EX", "MEM", "WB" };
	for (int i = mLastInstructions.size()-1; i >= 0; --i)
	{
		unsigned int addr = mLastInstructions[i];
		auto line = pAssembler->GetSourceLine(addr);
		std::cout << "\t0x" << std::hex << addr*4 << " " << STAGE[i] << std::dec << "  \t" << line << std::endl;
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

void Debugger::PrintOutputReg()
{
	long long output = GetRegisterVal(28);
	if (output != last_output_reg)
	{
		last_output_reg = output;
		std::cout << "OUTPUT REGISTER STORED: " << output << std::endl;
	}
}

void Debugger::PrintDataForward()
{
	if (pCPU->hazard.AluRsMux()[0].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardExMem().Read() << " from Ex/Mem to RS" << std::endl;
	}
	if (pCPU->hazard.AluRsMux()[1].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardMemWb().Read() << " from Mem/WB to RS" << std::endl;
	}
	if (pCPU->hazard.AluRtMux()[0].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardExMem().Read() << " from Ex/Mem to RT" << std::endl;
	}
	if (pCPU->hazard.AluRtMux()[1].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardMemWb().Read() << " from Mem/WB to RT" << std::endl;
	}
}
