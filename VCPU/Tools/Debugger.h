#pragma once

#include "CPU/CPU.h"
#include "Instruction.h"

class Debugger
{
public:
	Debugger(CPU& cpu);
	~Debugger();

	void LoadProgram();
	void Start();

private:
	void PrintInstruction();

	void PrintRegisters();
	
	CPU& mCPU;
	bool bPrintInstruction;
	bool bStep;

	std::vector<Instruction> program;
};

