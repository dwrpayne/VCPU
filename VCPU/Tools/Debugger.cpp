#include <iostream>
#include <string>
#include <sstream>
#include <bitset>
#include <iomanip>
#include "Debugger.h"
#include "Program.h" 
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
	bPrintBus = verbosity >= NORMAL; 

	pAssembler = new Assembler();
	pProgram = pAssembler->Assemble(source_filename);
	
	ProgramLoader loader(*pCPU);
	loader.Load(pProgram);
	pCPU->Connect();
}

Debugger::~Debugger()
{
	delete pCPU;
}

void Debugger::Start(int cycles)
{
	while (cycles != 0)
	{
		Step();
		if (bPrintInstruction)// && !pCPU->PipelineFreeze())
		{
			//__debugbreak();
		}
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
		if (pCPU->PipelineBubbleID())
		{
			mLastInstructions.insert(mLastInstructions.begin() + 1, -1);
		}
		else if (pCPU->PipelineBubbleEX())
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
		std::cout << "--------------------- CYCLE " << pCPU->cycles << " -----";
		std::cout << (pCPU->PipelineFreeze() ? " PIPELINE FREEZE" : "---------------") << "---------" << std::endl;
	}
	if (bPrintDataForward)
	{
		PrintDataForward();
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

	if (bPrintMemory)
	{
		PrintMemory();
	}

	if (bPrintBus)
	{
		PrintBus();
	}

	//if (bPrintTiming)
	{
		PrintTiming();
	}
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->Registers().registers[reg].Out().Read();
}

unsigned char Debugger::GetMemoryByte(int addr, bool cache, bool insmem)
{
	auto& cachesrc = insmem ? pCPU->InstructionCache() : pCPU->GetMainCache();
	auto& memsrc = insmem ? pCPU->InstructionMemory() : pCPU->GetMainMemory();
	if (cache)
	{
		unsigned int cacheline = addr / cachesrc.CACHE_LINE_BYTES;
		cacheline %= cachesrc.NUM_CACHE_LINES;
		auto line = cachesrc.cachelines[cacheline].OutLine();
		return line.Range<8>(8 * (addr % cachesrc.CACHE_LINE_BYTES)).Read();
	}
	else
	{
		unsigned int cacheline = addr / memsrc.CACHELINE_BYTES;
		cacheline %= memsrc.NUM_LINES;
		auto line = memsrc.cachelines[cacheline].Out();
		return line.Range<8>(8 * (addr % memsrc.CACHELINE_BYTES)).Read();
	}
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

std::string Debugger::GetMemoryString(int addr)
{
	std::stringstream ss;
	while (unsigned char c = GetMemoryByte(addr++))
	{
		ss << c;
	}
	return ss.str();
}

int Debugger::GetNextPCAddr()
{
	return pCPU->PC().UnsignedRead();
}

void Debugger::PrintInstruction()
{
	std::cout << "Current IR: " << std::bitset<32>(pCPU->IR().UnsignedRead()) << std::endl;
	std::cout << "Address  Stage   Assembled Instruction      Source Instruction  " << std::endl;
	static const char* STAGE[5] = { "IF", "ID", "EX", "MEM", "WB" };
	for (int i = mLastInstructions.size()-1; i >= 0; --i)
	{
		int addr = mLastInstructions[i];
		auto ass_line = addr >= 0 ? pProgram->GetAssembledLine(addr) : "inserted bubble";
		auto src_line = addr >= 0 ? pProgram->GetSourceLine(addr) : "inserted bubble";
		addr = std::max(addr, 0);
		
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
			std::cout << std::setw(2) << std::right << val;
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
	if (pCPU->hazardIFID.DoForwardRs().On())
	{
		if (pCPU->hazardIFID.ForwardExMemRs.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRs().Read() << " from Ex/Mem to ID stage RS" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRs().Read() << " from Mem/WB to ID stage RS" << std::endl;
		}
	}
	if (pCPU->hazardIFID.DoForwardRt().On())
	{
		if (pCPU->hazardIFID.ForwardExMemRt.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRt().Read() << " from Ex/Mem to ID stage RT" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRt().Read() << " from Mem/WB to ID stage RT" << std::endl;
		}
	}
	if (pCPU->hazardIDEX.DoForwardRs().On())
	{
		if (pCPU->hazardIDEX.ForwardExMemRs.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRs().Read() << " from Ex/Mem to EX stage RS (alu input A)" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRs().Read() << " from Mem/WB to EX stage RS (alu input A)" << std::endl;
		}
	}
	if (pCPU->hazardIDEX.DoForwardRt().On())
	{
		if (pCPU->hazardIDEX.ForwardExMemRt.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRt().Read() << " from Ex/Mem to EX stage RT (alu input B)" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRt().Read() << " from Mem/WB to EX stage RT (alu input B)" << std::endl;
		}
	}
}

void Debugger::PrintTiming()
{	
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
			int i = 0;
			for (auto us : pCPU->GetStageTiming())
			{
				std::cout << "CPU Stage " << i++ << " average " << us.count() << "us" << std::endl;
			}
			int imemcycle = pCPU->InstructionMemory().cycle;
			int mmemcycle = pCPU->GetMainMemory().cycle;
			long long imemus = pCPU->InstructionMemory().GetElapsedTime().count();
			long long mmemus = pCPU->GetMainMemory().GetElapsedTime().count();
			std::cout << "Instruction Mem updating at " << (1.0 * imemcycle) / ms << "kHz (avg time spent: " << imemus / imemcycle << "us)" << std::endl;
			std::cout << "Main Mem updating at " << (1.0 * mmemcycle) / ms << "kHz (avg time spent: " << mmemus / mmemcycle << "us)" << std::endl;
		}
	}
}

void Debugger::PrintBus()
{
	std::cout << "--- Addr | Ctrl: IGBKQWR (irq, grant, busreq, ack, req, write, read) ----- Data (first 100 bits) ----------" << std::endl;
	std::cout << std::hex << std::left << std::setw(8) << pCPU->GetSystemBus().OutAddr().UnsignedRead() << "    |    ";
	std::cout << std::bitset<SystemBus::Nctrl>(pCPU->GetSystemBus().OutCtrl().UnsignedRead()) << "     |    ";
	for (int i = 0; i < 100; i++)
	{
		std::cout << pCPU->GetSystemBus().OutData()[i];
	}
	std::cout << std::dec << std::endl;
}
