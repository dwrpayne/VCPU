#pragma once

#include "Component.h"
#include "ThreadedComponent.h"
#include "RegisterFile.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "PipelineBuffers.h"
#include "Cache.h"
#include "HazardUnit.h"
#include "Interlock.h"
#include "SystemBus.h"
#include "Controllers/DeviceController.h"

// Code mem runs 0x00000000 - 0x10000000
// User data starts at 0x10000000

// No Virt->Phys mapping yet, so stack starts at top of mem and grows down 
// 2KB main mem, so user stack starts at 0x10000800


class CPU : public Component
{
public:
	CPU();
	virtual ~CPU();

	int cycles;
	static const int WORD_SIZE = 32;
	static const int CACHE_LINE_BITS = 256;
	static const int NUM_REGISTERS = 32;

	static const int INS_CACHE_BYTES = 256;
	static const int INS_CACHE_BYTES = 128;
	static const int MAIN_CACHE_BYTES = 128;

	static const int INS_MEM_BYTES = 2048;
	static const int MAIN_MEM_BYTES = 16384;
		
	typedef Cache<INS_CACHE_BYTES, CACHE_LINE_BITS> InsCache;
	typedef Cache<MAIN_CACHE_BYTES, CACHE_LINE_BITS> MainCache;

	typedef Memory<CACHE_LINE_BITS, INS_MEM_BYTES> InsMemory;
	typedef Memory<CACHE_LINE_BITS, MAIN_MEM_BYTES> MainMemory;
	typedef RegisterFile<WORD_SIZE, NUM_REGISTERS> RegFile;

	void Connect();
	void Update();

private:

	const Bundle<32>& IR();
	const Bundle<32>& PC();
	InsCache& InstructionCache();
	MainCache& GetMainCache();
	InsMemory& InstructionMemory();
	MainMemory& GetMainMemory();
	SystemBus& GetSystemBus();
	RegFile& Registers();

	bool PipelineBubbleID() { return interlock.BubbleID().On(); }
	bool PipelineBubbleEX() { return interlock.BubbleEX().On(); }
	bool PipelineFreeze() { return interlock.Freeze().On(); }
	bool Halt();
	std::array<std::chrono::microseconds, 4> GetStageTiming();

private:
	class Stage1;
	class Stage2;
	class Stage3;
	class Stage4;

	Stage1* stage1;
	Stage2* stage2;
	Stage3* stage3;
	Stage4* stage4;

	InsMemory* mInsMemory;
	MainMemory* mMainMemory;
	KeyboardController mKeyboard;
	TerminalController mTerminal;
	
	HazardUnit hazardIDEX;
	HazardUnit hazardIFID;
	Interlock interlock;

	SystemBus systemBus;

	bool exit;

	friend class ProgramLoader;
	friend class Debugger;
};