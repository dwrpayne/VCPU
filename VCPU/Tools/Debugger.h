#pragma once
#include <chrono>
#include "CPU/CPU.h"
#include "Instruction.h"

class Debugger
{
public:
	Debugger(std::vector<Instruction>& program);

	void Start();
	void Step();
	int GetRegisterVal(int reg);
	int GetMemoryVal(int addr);
	int GetNextPCAddr();

	void SetInstructionPrint(bool state) { bPrintInstruction = state; }

private:
	void PrintInstruction();
	void PrintRegisters();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;

	std::chrono::microseconds mCpuElapsedTime;

	std::vector<Instruction> mProgram;
};

