#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "Debugger.h"
#include "ProgramLoader.h" 
#include "Assembler.h"


Debugger::Debugger(const std::string& source_filename, Verbosity verbosity)
{
	pCPU = new CPU();
	bPrintInstruction = verbosity >= NORMAL;
	bPrintRegisters = verbosity >= NORMAL;
	bPrintMemory = verbosity >= MEMORY;
	bPrintOutputReg = verbosity >= MINIMAL;
	bPrintDataForward = verbosity >= VERBOSE;
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

	if (bPrintTiming)
	{
		for (int i = 0; i < NUM_BUCKETS; i++)
		{
			if (mThisCycleTime.count() < BUCKETS[i])
			{
				cycleTimeBuckets[i]++;
				break;
			}
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

	if (bPrintMemory)
	{
		PrintMemory();
	}

	if (bPrintTiming)
	{
		PrintTiming();
	}
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->Registers().registers[reg].Out().Read();
}

unsigned char Debugger::GetMemoryByte(int addr)
{
	return pCPU->MainMem().mMemory.registers[addr].Out().Read();
}

int Debugger::GetMemoryWord(int addr)
{
	int word = 0;
	for (int i = 0; i < 4; i++)
	{
		word |= (GetMemoryByte(addr + i) << (i * 8));
	}
	return word;
}

int Debugger::GetNextPCAddr()
{
	return pCPU->PC().UnsignedRead();
}

void Debugger::PrintInstruction()
{
	std::cout << "--------------------- CYCLE " << pCPU->cycles << " -----";
	std::cout << (pCPU->PipelineFreeze() ? " PIPELINE FREEZE" : "---------------") << "---------" << std::endl;
	std::cout << "Address  Stage   Assembled Instruction      Source Instruction" << std::endl;
	static const char* STAGE[5] = { "IF", "ID", "EX", "MEM", "WB" };
	for (int i = mLastInstructions.size()-1; i >= 0; --i)
	{
		int addr = mLastInstructions[i];
		auto ass_line = addr >= 0 ? pAssembler->GetAssembledLine(addr) : "inserted bubble";
		auto src_line = addr >= 0 ? pAssembler->GetSourceLine(addr) : "";

		std::stringstream ss;
		ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << addr * 4;
		ss << std::setfill(' ') << std::left << " " << std::setw(5) << STAGE[i] << "|  ";
		ss << std::setw(25) << ass_line << "| " << src_line << std::endl;
		std::cout << ss.str();
	}
}

void Debugger::PrintRegisters()
{
	std::cout << std::endl;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int num = i + 8 * j;
			std::stringstream ss;
			ss << "$" << pAssembler->GetRegName(num) << "(" << num << ") ";
			std::cout << std::left << std::setw(8) << ss.str();
			std::cout << std::setw(12) << GetRegisterVal(i + 8 * j);
		}
		std::cout << std::endl;
	}
}

void Debugger::PrintMemory()
{
	bool diff = false;
	for (unsigned int i = 0; i < mLastCycleMemory.size(); i++)
	{
		if (mLastCycleMemory[i] != GetMemoryByte(i))
		{
			diff = true;
		}
		mLastCycleMemory[i] = GetMemoryByte(i);
	}
	if (diff)
	{
		std::cout << "Memory: " << std::hex << std::setfill('0');
		for (unsigned short val : mLastCycleMemory)
		{
			std::cout << std::setw(2) << val;
		}
		std::cout << std::dec << std::setfill(' ') << std::endl;
	}
}

void Debugger::PrintOutputReg()
{
	long long output = GetRegisterVal(28);
	if (output != last_output_reg)
	{
		last_output_reg = output;
		std::cout << "OUTPUT: " << output << " at cycle " << pCPU->cycles << std::endl;
	}
}

void Debugger::PrintDataForward()
{
	if (pCPU->hazard.AluRsMux()[0].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardExMem().Read() << " from Ex/Mem to EX stage RS (alu input A)" << std::endl;
	}
	if (pCPU->hazard.AluRsMux()[1].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardMemWb().Read() << " from Mem/WB to EX stage RS (alu input A)" << std::endl;
	}
	if (pCPU->hazard.AluRtMux()[0].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardExMem().Read() << " from Ex/Mem to EX stage RT (alu input B)" << std::endl;
	}
	if (pCPU->hazard.AluRtMux()[1].On())
	{
		std::cout << "Forwarding " << pCPU->hazard.ForwardMemWb().Read() << " from Mem/WB to EX stage RT (alu input B)" << std::endl;
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

		if (bPrintTiming)
		{
			for (int i = 0; i < NUM_BUCKETS; i++)
			{
				std::cout << BUCKETS[i] << "\t" << cycleTimeBuckets[i] << std::endl;
			}
		}
	}
}