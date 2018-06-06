#pragma once
#include <chrono>
#include <deque>
#include "CPU/CPU.h"

class Debugger
{
public:
	enum Verbosity
	{
		SILENT,
		MINIMAL,
		TIMING,
		NORMAL,
		VERBOSE
	};
	Debugger(const std::string& source_filename, Verbosity verbosity = NORMAL);

	void Start(int cycles = -1);
	void Step();
	void PrintCycle();
	int GetRegisterVal(int reg);
	int GetMemoryVal(int addr);
	int GetNextPCAddr();

	void SetInstructionPrint(bool state) { bPrintInstruction = state; }
	void SetRegisterPrint(bool state) { bPrintRegisters = state; }
	void SetOutputRegPrint(bool state) { bPrintOutputReg = state; }
	void SetDataForwardPrint(bool state) { bPrintDataForward = state; }
	void SetTimingPrint(bool state) { bPrintTiming = state; }

private:
	void PrintInstruction();
	void PrintRegisters();
	void PrintOutputReg();
	void PrintDataForward();
	void PrintTiming();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;
	bool bPrintOutputReg;
	bool bPrintDataForward;
	bool bPrintTiming;

	std::deque<int> mLastInstructions;

	long long last_output_reg;

	std::chrono::microseconds mThisCycleTime;
	std::chrono::microseconds mCpuElapsedTime;

	static const int NUM_BUCKETS = 24;
	int cycleTimeBuckets[NUM_BUCKETS];
	inline static const int BUCKETS[NUM_BUCKETS] = {
		130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 190, 200, 225, 250, 275, 300, 400, 500, 750, 1000, 2000, 5000, 1000000 };

	class Assembler* pAssembler;
};

