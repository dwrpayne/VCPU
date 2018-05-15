#pragma once

#include "CPU/CPU.h"

class Debugger
{
public:
	Debugger(CPU& cpu);
	~Debugger();

	void LoadProgram();

private:
	CPU& mCPU;
};

