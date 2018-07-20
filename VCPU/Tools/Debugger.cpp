#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>
#include <iomanip>
#include "Debugger.h"
#include "Program.h" 
#include "ProgramLoader.h" 
#include "Assembler.h"
#include "CPU/Addresses.h"


Debugger::Debugger(const std::string& source_filename, Verbosity verbosity)
	: cur_filename(source_filename)
{
	pCPU = new CPU();
	bSingleStep = verbosity >= NORMAL;
	bPrintInstruction = verbosity >= NORMAL;
	bPrintRegisters = verbosity >= NORMAL;
	bPrintMemory = verbosity >= VERBOSE;
	bPrintDataForward = verbosity >= VERBOSE;
	bPrintTiming = verbosity >= TIMING;
	bPrintStack = verbosity >= NORMAL;
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
		//pCPU->GetSystemBus().WriteBus();
		if (bSingleStep && !pCPU->PipelineFreeze())
		{
			__debugbreak();
		}
 		if (pCPU->Break() && !pCPU->PipelineFreeze())
		{
			//SaveMemoryToDisk();
			PrintTiming(true);
			//__debugbreak();
		}
 		if (pCPU->Halt())
		{
			SaveMemoryToDisk();
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
	pCPU->PostUpdate();
	
	mCpuElapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

	if (pCPU->Fault())
	{
		bSingleStep = true;
		bPrintInstruction = true;
		bPrintRegisters = true;
		bPrintDataForward = true;
		bPrintStack = true;
		bPrintBus = true;
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

		DoProfilingCheck();
	}

	PrintCycle();
}

void Debugger::PrintCycle()
{
	//bPrintInstruction = pCPU->Break();
	//bPrintRegisters = pCPU->Break();

	//if (!pCPU->PipelineFreeze())
	{
		if (bPrintInstruction)
		{
			std::cout << "---------- INSTRUCTION " << pCPU->instructions << " ------ ( CYCLE " << pCPU->cycles << " ) -------";
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

		if (bPrintStack)
		{
			PrintStack();
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
	if (bPrintBus)
	{
		PrintBus();
	}
}

int Debugger::GetRegisterVal(int reg)
{
	return pCPU->Registers().registers[reg].Out().Read();
}

unsigned char Debugger::GetCacheByte(int addr)
{
	auto& cache = pCPU->GetMainCache();
	unsigned int cacheline = addr / cache.CACHE_LINE_BYTES;
	cacheline %= cache.NUM_CACHE_LINES;
	auto tag = cache.cachelines[cacheline].Tag();
	auto valid = cache.cachelines[cacheline].Valid();

	if (valid.On() && (tag.UnsignedRead() == Bundle<32>(addr).Range<CPU::MainCache::TAG_BITS>(CPU::MainCache::ADDR_BITS - CPU::MainCache::TAG_BITS).UnsignedRead()))
	{
		auto line = cache.cachelines[cacheline].OutLine();
		return line.Range<8>(8 * (addr % cache.CACHE_LINE_BYTES)).Read();
	}

	return GetMemoryByte(addr);
}

unsigned char Debugger::GetMemoryByte(int addr)
{
	if (addr >= USER_DATA_START)
	{
		addr -= USER_DATA_START;
	}
	auto& mem = pCPU->GetMainMemory();
	unsigned int cacheline = addr / mem.CACHELINE_BYTES;
	auto line = mem.cachelines[cacheline].Out();
	return line.Range<8>(8 * (addr % mem.CACHELINE_BYTES)).Read();
}

unsigned int Debugger::GetCacheWord(int addr)
{
	unsigned int word = 0;
	for (int i = 0; i < 4; i++)
	{
		word |= (unsigned int)(GetCacheByte(addr + i) << (i * 8));
	}
	return word;
}

unsigned int Debugger::GetMemoryWord(int addr)
{
	unsigned int word = 0;
	for (int i = 0; i < 4; i++)
	{
		word |= (unsigned int)(GetMemoryByte(addr + i) << (i * 8));
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

void Debugger::SaveMemoryToDisk()
{
	for (int i = 0; i < 1000; i++)
	{
		std::string filename = cur_filename + std::to_string(i) + ".bin";
		std::ifstream ifs(filename);
		if (ifs.is_open()) continue;

		std::ofstream of(filename);
		for (int i = USER_DATA_START; i < KERNEL_DATA_END; i++)
		{
			of << GetCacheByte(i);
		}
		break;
	}
}

void Debugger::PrintInstruction()
{
	//assert(pCPU->IR().UnsignedRead() == pProgram->GetInstruction(mLastInstructions[0])->mBinary);
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
			unsigned int val = GetRegisterVal(num);
			std::stringstream ss;
			ss << "$" << pAssembler->GetRegName(num) << "(" << num << ") ";
			std::cout << std::left << std::setw(8) << ss.str();

			if (num < 28 && val < 0x10000000)
			{
				std::cout << std::setw(12) << GetRegisterVal(num);
			}
			else
			{
				std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << std::right << GetRegisterVal(num) << "  ";
				std::cout << std::setw(12) << std::setfill(' ') << std::dec;
			}
		}
		std::cout << std::endl;
	}
}

void Debugger::PrintMemory(bool force)
{
	bool diff = false;
	for (unsigned int i = 0; i < mLastCycleMemory.size(); i++)
	{
		if (mLastCycleMemory[i] != GetMemoryByte(i))
		{
			std::cout << "Byte " << i << ": Last cycle was " << (int)mLastCycleMemory[i] << ", now it's " << (int)GetMemoryByte(i) << std::endl;
			diff = true;
		}
		mLastCycleMemory[i] = GetMemoryByte(i);
	}
	if (diff || force)
	{
		std::cout << "Memory: " << std::hex << std::setfill('0');
		int i = 0;
		for (unsigned short val : mLastCycleMemory)
		{
			if (!(i++ % 64))
			{
				std::cout << std::endl << "0x" << std::hex << std::setfill('0') << std::setw(8) << std::right << i-1 << "  ";
			}
			std::cout << std::setw(2) << std::right << val;
			if (!(i % 16))
			{
				std::cout << " ";
			}
		}
		std::cout << std::dec << std::setfill(' ') << std::endl;
	}
}

void Debugger::PrintDataForward()
{
	if (pCPU->hazardIFID.DoForwardRs().On())
	{
		if (pCPU->hazardIFID.ForwardExMemRs.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRs().Read() << 
				"(0x" << std::hex << pCPU->hazardIFID.ForwardDataRs().Read() << std::dec << ") from Ex/Mem to ID stage RS" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRs().Read() << 
				"(0x" << std::hex << pCPU->hazardIFID.ForwardDataRs().Read() << std::dec << ") from Mem/WB to ID stage RS" << std::endl;
		}
	}
	if (pCPU->hazardIFID.DoForwardRt().On())
	{
		if (pCPU->hazardIFID.ForwardExMemRt.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRt().Read() << 
				"(0x" << std::hex << pCPU->hazardIFID.ForwardDataRt().Read() << std::dec << ") from Ex/Mem to ID stage RT" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIFID.ForwardDataRt().Read() << 
				"(0x" << std::hex << pCPU->hazardIFID.ForwardDataRt().Read() << std::dec << ") from Mem/WB to ID stage RT" << std::endl;
		}
	}
	if (pCPU->hazardIDEX.DoForwardRs().On())
	{
		if (pCPU->hazardIDEX.ForwardExMemRs.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRs().Read() << 
				"(0x" << std::hex << pCPU->hazardIDEX.ForwardDataRs().Read() << std::dec << ") from Ex/Mem to EX stage RS (alu input A)" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRs().Read() << 
				"(0x" << std::hex << pCPU->hazardIDEX.ForwardDataRs().Read() << std::dec << ") from Mem/WB to EX stage RS (alu input A)" << std::endl;
		}
	}
	if (pCPU->hazardIDEX.DoForwardRt().On())
	{
		if (pCPU->hazardIDEX.ForwardExMemRt.Out().On())
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRt().Read() << 
				"(0x" << std::hex << pCPU->hazardIDEX.ForwardDataRt().Read() << std::dec << ") from Ex/Mem to EX stage RT (alu input B)" << std::endl;
		}
		else
		{
			std::cout << "Forwarding " << pCPU->hazardIDEX.ForwardDataRt().Read() << 
				"(0x" << std::hex << pCPU->hazardIDEX.ForwardDataRt().Read() << std::dec << ") from Mem/WB to EX stage RT (alu input B)" << std::endl;
		}
	}
	if (pCPU->hazardEXMEM.DoForwardRt().On())
	{
		std::cout << "Forwarding " << pCPU->hazardEXMEM.ForwardDataRt().Read() << 
			"(0x" << std::hex << pCPU->hazardEXMEM.ForwardDataRt().Read() << std::dec << ") from Mem/Wb to MEM stage RT (mem write value)" << std::endl;
	}
}

void Debugger::PrintStack()
{
	unsigned int cur_stack = pCPU->Registers().registers[29].Out().UnsignedRead();
	if (USER_DATA_START <= cur_stack && cur_stack <= USER_STACK_START)
	{
		if (cur_stack != mLastStackPointer)
		{
			mLastStackPointer = cur_stack;
			std::cout << "----- Stack print: top 0x" << std::hex << cur_stack << " ------------------" << std::endl;
			for (int addr = cur_stack; addr < USER_STACK_START; addr += 4)
			{
				unsigned int val = GetCacheWord(addr);
				std::cout << "0x" << std::hex << addr << "\t";
				if (val < USER_DATA_START)
				{
					std::cout << std::dec;
				}
				std::cout << val << std::dec << "\t" << pProgram->GetSourceLine(val/4 - 2) << std::endl;
			}
			std::cout << "-------------------------------" << std::endl << std::endl;
		}
	}
}

void Debugger::PrintTiming(bool force)
{	
	if (force || pCPU->instructions % 10000 == 0)
	{
		std::stringstream ss;
		long long ms = mCpuElapsedTime.count() / 1000;
		ss << "------------------- Timing details for cycle " << pCPU->cycles << "------------------" << std::endl;
		ss << pCPU->cycles << " cycles and " << pCPU->instructions << " instructions  in " << ms / 1000.0 << "sec." << std::endl;
		ss << "Average clock freq of " << (1.0 * pCPU->cycles) / ms << "kHz" << std::endl;
		ss << "Average of " << (1.0 * pCPU->cycles) / pCPU->instructions << " cycles / instructions." << std::endl;
		if (pCPU->ins_cachemisses)
		{
			ss << "Instruction Cache Misses" << std::endl;
			ss << "\t\tTotal count: " << pCPU->ins_cachemisses << std::endl;
			ss << "\t\tCycles per miss: " << 1.0 * pCPU->ins_cachemiss_cycles / pCPU->ins_cachemisses << std::endl;
			ss << "\t\t% of instructions that missed: " << (100.0*pCPU->ins_cachemisses) / pCPU->instructions << std::endl;
		}
		if (pCPU->data_cachemisses)
		{
			ss << "Data Cache Misses" << std::endl;
			ss << "\t\tTotal count: " << pCPU->data_cachemisses << std::endl;
			ss << "\t\tCycles per miss: " << 1.0 * pCPU->data_cachemiss_cycles / pCPU->data_cachemisses << std::endl;
			ss << "\t\t% of instructions that missed: " << (100.0*pCPU->data_cachemisses) / pCPU->instructions << std::endl;
		}

		if (force || bPrintTiming)
		{
			ss << "CPU update average " << mCpuElapsedTime.count() / pCPU->cycles << "us. Breakdown:" << std::endl;
			int i = 0;
			for (auto stage_us : pCPU->GetStageTiming())
			{
				ss << "CPU Stage " << i++ << " average " << stage_us.count() << "us" << std::endl;
			}
			int imemcycle = pCPU->InstructionMemory().cycle;
			int mmemcycle = pCPU->GetMainMemory().cycle;
			long long imemus = pCPU->InstructionMemory().GetElapsedTime().count();
			long long mmemus = pCPU->GetMainMemory().GetElapsedTime().count();
			ss << "Instruction Mem updating at " << (1.0 * imemcycle) / ms << "kHz (avg time spent: " << imemus / imemcycle << "us)" << std::endl;
			ss << "Main Mem updating at " << (1.0 * mmemcycle) / ms << "kHz (avg time spent: " << mmemus / mmemcycle << "us)" << std::endl;
		}
		std::cout << ss.str();
	}
}

void Debugger::PrintBus()
{
	pCPU->GetSystemBus().PrintBus();
}

void Debugger::DoProfilingCheck()
{
	auto codeline = pProgram->GetLine(mLastInstructions[0]);
	bool start = !mIsCurrentlyProfiling && codeline->mComment.find("!PROFILE START") != std::string::npos;
	bool stop = mIsCurrentlyProfiling && codeline->mComment.find("!PROFILE STOP") != std::string::npos;

	if (start) mIsCurrentlyProfiling = true;
	if (stop) mIsCurrentlyProfiling = false;

	if (mIsCurrentlyProfiling)
	{
		mProfilingLineCounts[codeline] += 1;
	}
	if (stop)
	{
		for (auto &[line, count] : mProfilingLineCounts)
		{
			std::cout << std::setw(60) << std::left << line->to_string();
			std::cout << "\t\t" << count << std::endl;
		}
	}
}

