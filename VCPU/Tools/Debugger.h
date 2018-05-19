#pragma once

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

private:
	void PrintInstruction();
	void PrintRegisters();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bPrintRegisters;

	std::vector<Instruction> mProgram;
};

