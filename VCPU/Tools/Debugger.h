#pragma once

#include "CPU/CPU.h"
#include "Instruction.h"

class Debugger
{
public:
	Debugger(std::vector<Instruction>& program);

	void Start();

private:
	void PrintInstruction();
	void PrintRegisters();
	
	CPU* pCPU;
	bool bPrintInstruction;
	bool bStep;

	std::vector<Instruction> mProgram;
};

