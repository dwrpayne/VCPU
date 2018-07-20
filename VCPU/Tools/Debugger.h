#pragma once
#include <chrono>
#include <deque>
#include <array>
#include <map>

class Assembler;
class Program;
class CPU;

class Debugger
{
public:
	enum Verbosity
	{
		SILENT,
		MINIMAL,
		TIMING,
		MEMORY,
		NORMAL,
		VERBOSE,
	};
	Debugger(const std::string& source_filename, Verbosity verbosity = NORMAL);
	~Debugger();

	void Start(int cycles = -1);
	void Step();
	void PrintCycle();
	int GetRegisterVal(int reg);
	unsigned char GetCacheByte(int addr);
	unsigned char GetMemoryByte(int addr);
	unsigned int GetCacheWord(int addr);
	unsigned int GetMemoryWord(int addr);
	std::string GetMemoryString(int addr);
	int GetNextPCAddr();
	
private:
	void SaveMemoryToDisk();

	void PrintInstruction();
	void PrintRegisters();
	void PrintMemory(bool force=false);
	void PrintDataForward();
	void PrintStack();
	void PrintTiming(bool force=false);
	void PrintBus();

	void DoProfilingCheck();
	
	CPU* pCPU;
	bool bSingleStep;
	bool bPrintInstruction;
	bool bPrintRegisters;
	bool bPrintMemory;
	bool bPrintDataForward;
	bool bPrintTiming;
	bool bPrintStack;
	bool bPrintBus;

	std::string cur_filename;

	std::deque<int> mLastInstructions;
	std::array<unsigned char, 16*1024> mLastCycleMemory;
	unsigned int mLastStackPointer;

	bool mIsCurrentlyProfiling;
	std::map<const class CodeLine*, int> mProfilingLineCounts;
	 
	std::chrono::microseconds mCpuElapsedTime;

	Assembler* pAssembler;
	const Program* pProgram;
};

