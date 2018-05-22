#pragma once

#include "Component.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "PipelineBuffers.h"
#include "Cache.h"

class CPU : public Component
{
public:
	void Connect();
	void Update();

	int cycles;

	typedef Memory<32, 128, 256> InsMemory;
	typedef Memory<32, 128, 256> MainMemory; // Todo: cache levels
	typedef RegisterFile<32, 32> RegFile;

	
private:
	void Update1();
	void Update2();
	void Update3();
	void Update4();
	void Update5();

	MuxBundle<32, 2> pcInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
	
	Cache instructionCache;
	InsMemory instructionMem;
	BufferIFID bufIFID;
	
	OpcodeDecoder opcodeControl;
	RegFile regFile;

	BufferIDEX bufIDEX;

	FullAdderN<32> pcJumpAdder;
	MuxBundle<32, 2> aluBInputMux;
	ALU<32> alu;
	MuxBundle<RegFile::ADDR_BITS, 2> regFileWriteAddrMux;

	BufferEXMEM bufEXMEM;

	// TODO: Extract these gates into a Component
	Inverter aluZeroInv;
	OrGate aluNegOrZero;
	Inverter aluPos;
	Multiplexer<4> branchTakenMux;
	AndGate branchTakenAnd;

	Cache cache;
	MainMemory mainMem;

	BufferMEMWB bufMEMWB;

	MuxBundle<32, 4> regWriteDataMux;

	friend class ProgramLoader;
	friend class Debugger;
};