#include <iostream>
#include <string>
#include "Debugger.h"
#include "ProgramLoader.h" 
#include "Assembler.h"


Debugger::Debugger(const std::string& source_filename, Verbosity verbosity)
{
	pCPU = new CPU();
	bPrintInstruction = verbosity >= NORMAL;
	bPrintRegisters = verbosity >= VERBOSE;
	bPrintOutputReg = verbosity >= MINIMAL;
	bPrintDataForward = verbosity >= NORMAL;

	pAssembler = new Assembler(source_filename);


	ProgramLoader loader(*pCPU);
	loader.Load(pAssembler->GetBinary());
	pCPU->Connect();
}

void Debugger::Start(int cycles)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	while (cycles != 0)
	{
		Step();

		if (pCPU->Halt())
		{
			break;
		}

		if (pCPU->cycles % 10000 == 0)
		{
			auto now_time = std::chrono::high_resolution_clock::now();
			long long ms = std::chrono::duration_cast<std::chrono::microseconds>(now_time-start_time).count() / 1000;
			std::cout << pCPU->cycles << " cycles in " << ms/1000.0 << "sec. Average clock freq of " << (1.0 * pCPU->cycles) / ms << "kHz" << std::endl;
		}
		cycles--;
	}
}

void Debugger::Step()
{
	//auto t1 = std::chrono::high_resolution_clock::now();
	pCPU->Update();
	//auto t2 = std::chrono::high_resolution_clock::now();
	
	//mCpuElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	int word = GetNextPCAddr() / 4;
	mLastInstructions.push_front({ word, pCPU->PipelineBubble(), pCPU->PipelineFreeze() });
	if (mLastInstructions.size() > 5)
	{
		mLastInstructions.pop_back();
	}

	if (bPrintInstruction)
	{
		PrintInstruction();
	}

	if (bPrintDataForward)
	{
		PrintDataForward();
	}
	
	if (bPrintRegisters)
	{
		PrintRegisters();
	}

	if (bPrintOutputReg)
	{
		PrintOutputReg();
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
	return pCPU->PC().UnsignedRead();
}

void Debugger::PrintInstruction()
{
	std::cout << "--------- CYCLE " << pCPU->cycles << " ---------------" << std::endl;
	static const char* STAGE[5] = { "IF", "ID", "EX", "MEM", "WB" };
	for (int i = mLastInstructions.size()-1; i >= 0; --i)
	{
		auto [addr, bubble, freeze] = mLastInstructions[i];
		auto line = pAssembler->GetSourceLine(addr);
		if (bubble)
		{
			line.append(" (Bubble stall)");
		}
		else if (freeze)
		{
			line.append(" (Pipeline freeze)");
		}
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
