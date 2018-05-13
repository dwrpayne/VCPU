#pragma once

#include "Component.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"

#include "InsRegister.h"

class CPU : public Component
{
public:
	void Connect();
	void Update();

	int cycles;

	typedef Memory<32, 128> InsMemory;
	typedef Memory<32, 1024> MainMemory; // Todo: cache levels
	typedef RegisterFile<32, 32> RegFile;

	void ConnectToLoader(Bundle<32>& addr, Bundle<32> ins);
	void LoadInstruction();

private:
	MuxBundle<32, 2> pcInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
	FullAdderN<32> pcJumpAdder;

	InsMemory instructionMem;
	InsRegister ir;
	MuxBundle<RegFile::ADDR_BITS, 2> regFileWriteAddrMux;
	RegFile regFile;

	MuxBundle<32, 2> aluBInputMux;
	ALU<32> alu;
	Register<32> aluOut;

	MainMemory mainMem;
	MuxBundle<32, 2> regWriteDataMux;
};