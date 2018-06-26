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
	unsigned char GetMemoryByte(int addr);
	int GetMemoryWord(int addr);
	std::string GetMemoryString(int addr);
	int GetNextPCAddr();
	
private:
	void PrintInstruction();
	void PrintRegisters();
	void PrintMemory();
	void PrintOutputReg();
	void PrintDataForward();
	void PrintTiming();
	void PrintBus();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;
	bool bPrintMemory;
	bool bPrintOutputReg;
	bool bPrintDataForward;
	bool bPrintTiming;
	bool bPrintBus;

	std::deque<int> mLastInstructions;
	std::array<unsigned char, 64> mLastCycleMemory;

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

