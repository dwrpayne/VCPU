#pragma once

#include "Component.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"

class CPU : public Component
{
public:
	void Connect();
	void Update();

	typedef Memory<32, 128> InsMemory;

private:
	MuxBundle<32, 2> pcInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
	FullAdderN<32> pcJumpAdder;

	InsMemory instructionMem;
	Register<32> ir;
	Register<32> mdr;
	RegisterFile<32, 32> registers;
	ALU<32> alu;
	Register<32> aluOut;
};