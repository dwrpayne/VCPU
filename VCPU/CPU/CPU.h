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
// 16KB memory. Let's reserve 4KB for the kernel, and 12KB for user.
// user stack starts at 01x10002fff and grows down.
// kernel mem runs 0x10003000 - 01x10003fff



class CPU : public Component
{
public:
	CPU();
	virtual ~CPU();

	int cycles;
	int instructions;
	int ins_cachemisses;
	int ins_cachemiss_cycles;
	int data_cachemisses;
	int data_cachemiss_cycles;

	static const int WORD_SIZE = 32;
	static const int CACHE_LINE_BITS = 256;
	static const int NUM_REGISTERS = 32;

	static const int INS_CACHE_BYTES = 512;
	static const int MAIN_CACHE_BYTES = 512;

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
	bool Break();
	std::array<std::chrono::microseconds, 4> GetStageTiming();

private:
	// Declare first so it gets destroyed last 
	// Give components a chance to disconnect cleanly first
	SystemBus systemBus;

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

	bool mIsMissingInsCache;
	bool mIsMissingDataCache;
	
	bool exit;

	friend class ProgramLoader;
	friend class Debugger;
};