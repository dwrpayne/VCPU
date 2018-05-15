#pragma once

#include "CPU/CPU.h"

class Debugger
{
public:
	Debugger(CPU& cpu);
	~Debugger();

	void LoadProgram();
	void Start();

private:
	CPU& mCPU;
};

