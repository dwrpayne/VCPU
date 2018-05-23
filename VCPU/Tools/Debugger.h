#pragma once
#include <chrono>
#include "CPU/CPU.h"

class Debugger
{
public:
	Debugger(const std::string& source_filename);

	void Start();
	void Step();
	int GetRegisterVal(int reg);
	int GetMemoryVal(int addr);
	int GetNextPCAddr();

	void SetInstructionPrint(bool state) { bPrintInstruction = state; }
	void SetRegisterPrint(bool state) { bPrintRegisters = state; }

private:
	void PrintInstruction();
	void PrintRegisters();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;

	std::chrono::microseconds mCpuElapsedTime;

	class Assembler* pAssembler;
};

