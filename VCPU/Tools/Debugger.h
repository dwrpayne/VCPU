#pragma once
#include <chrono>
#include <deque>
#include "CPU/CPU.h"

class Assembler;
class Program;

class Debugger
{
public:
	enum Verbosity
	{
		SILENT,
		MINIMAL,
		MEMORY,
		TIMING,
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
	void PrintTiming();
	void PrintBus();
	
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

	long long last_output_reg;

	std::chrono::microseconds mThisCycleTime;
	std::chrono::microseconds mCpuElapsedTime;

	static const int NUM_BUCKETS = 17;
	int cycleTimeBuckets[NUM_BUCKETS];
	inline static const int BUCKETS[NUM_BUCKETS] = {
		100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 225, 250, 500, 1000, 5000, 1000000 };

	Assembler* pAssembler;
	const Program* pProgram;
};

