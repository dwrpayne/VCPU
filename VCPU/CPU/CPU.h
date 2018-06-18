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


class CPU : public Component
{
public:
	CPU();
	virtual ~CPU();

	int cycles;
	static const int WORD_SIZE = 32;
	static const int CACHE_LINE_BITS = 256;
	static const int NUM_REGISTERS = 32;

	static const int INS_CACHE_BYTES = 128;
	static const int MAIN_CACHE_BYTES = 128;

	static const int INS_MEM_BYTES = 1024;
	static const int MAIN_MEM_BYTES = 1024;
		
	typedef Cache<INS_CACHE_BYTES, CACHE_LINE_BITS, INS_MEM_BYTES> InsCache;
	typedef Cache<MAIN_CACHE_BYTES, CACHE_LINE_BITS, MAIN_MEM_BYTES> MainCache;
	typedef InsCache::MemoryType InsMemory;
	typedef MainCache::MemoryType MainMemory;
	typedef RegisterFile<WORD_SIZE, NUM_REGISTERS> RegFile;

	void Connect();
	void Update();

private:

	const Bundle<32>& PC();
	InsCache& InstructionMem();
	MainCache& MainMem();
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
	
	std::condition_variable mCV;
	std::mutex mMutex;
	bool stage1Ready;
	bool stage2Ready;
	bool stage3Ready;
	bool stage4Ready;
	std::thread stage1Thread;
	std::thread stage2Thread;
	std::thread stage3Thread;
	std::thread stage4Thread;

	HazardUnit hazardIDEX;
	HazardUnit hazardIFID;
	Interlock interlock;

	bool exit;

	friend class ProgramLoader;
	friend class Debugger;
};