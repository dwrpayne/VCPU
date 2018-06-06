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
	bPrintTiming = verbosity >= TIMING;

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

		if (pCPU->Halt())
		{
			break;
		}
		cycles--;
	}
}

void Debugger::Step()
{
	auto t1 = std::chrono::high_resolution_clock::now();
	pCPU->Update();
	auto t2 = std::chrono::high_resolution_clock::now();
	
	mThisCycleTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
	mCpuElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	for (int i = 0; i < NUM_BUCKETS; i++)
	{
		if (mThisCycleTime.count() < BUCKETS[i])
		{
			cycleTimeBuckets[i]++;
			break;
		}
	}

	int word = GetNextPCAddr() / 4;
	if (!pCPU->PipelineFreeze())
	{
		if (pCPU->PipelineBubble())
		{
			mLastInstructions.insert(mLastInstructions.begin() + 2, -1);
		}
		else
		{
			mLastInstructions.push_front({ word });
		}

		if (mLastInstructions.size() > 5)
		{
			mLastInstructions.pop_back();
		}
	}

	PrintCycle();
}

void Debugger::PrintCycle()
{
	if (bPrintInstruction)
	{
		PrintInstruction();
		if (pCPU->PipelineFreeze())
		{
			std::cout << "PIPELINE FREEZE THIS CYCLE" << std::endl;
		}
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
	
	PrintTiming();
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
		int addr = mLastInstructions[i];
		auto line = addr > 0 ? pAssembler->GetSourceLine(addr) : "inserted bubble";
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

void Debugger::PrintTiming()
{	
	if (bPrintTiming)
	{
		std::cout << "Cycle time: " << mThisCycleTime.count() << "us" << std::endl;
	}
	
	if (pCPU->cycles % 10000 == 0)
	{
		long long ms = mCpuElapsedTime.count() / 1000;
		std::cout << pCPU->cycles << " cycles in " << ms / 1000.0 << "sec.";
		std::cout << " Average clock freq of " << (1.0 * pCPU->cycles) / ms << "kHz" << std::endl;
		std::cout << "Time(us)  Count" << std::endl;
		for (int i = 0; i < NUM_BUCKETS; i++)
		{
			std::cout << BUCKETS[i] << "\t" << cycleTimeBuckets[i] << std::endl;
		}
	}
}