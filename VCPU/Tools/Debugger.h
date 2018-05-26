#pragma once
#include <chrono>
#include <deque>
#include "CPU/CPU.h"

class Debugger
{
public:
	Debugger(const std::string& source_filename);

	void Start(int cycles = -1);
	void Step();
	int GetRegisterVal(int reg);
	int GetMemoryVal(int addr);
	int GetNextPCAddr();

	void SetInstructionPrint(bool state) { bPrintInstruction = state; }
	void SetRegisterPrint(bool state) { bPrintRegisters = state; }
	void SetOutputRegPrint(bool state) { bPrintOutputReg = state; }
	void SetDataForwardPrint(bool state) { bPrintOutputReg = state; }

private:
	void PrintInstruction();
	void PrintRegisters();
	void PrintOutputReg();
	void PrintDataForward();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;
	bool bPrintOutputReg;
	bool bPrintDataForward;

	std::deque<int> mLastInstructions;

	long long last_output_reg;

	std::chrono::microseconds mCpuElapsedTime;

	class Assembler* pAssembler;
};

