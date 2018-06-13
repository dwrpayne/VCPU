#pragma once

#include "Component.h"
#include "RegisterFile.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "BranchDetector.h"
#include "PipelineBuffers.h"
#include "Cache.h"
#include "HazardUnit.h"
#include "Interlock.h"


class CPU : public Component
{
public:
	CPU();

	int cycles;
	static const int WORD_SIZE = 32;
	static const int CACHE_LINE_BITS = 256;
	static const int NUM_REGISTERS = 32;

	static const int INS_CACHE_BYTES = 128;
	static const int MAIN_CACHE_BYTES = 256;

	static const int INS_MEM_BYTES = 512;
	static const int MAIN_MEM_BYTES = 2048;
		
	typedef Cache<WORD_SIZE, INS_CACHE_BYTES, CACHE_LINE_BITS, INS_MEM_BYTES> InsCache;
	typedef Cache<WORD_SIZE, MAIN_CACHE_BYTES, CACHE_LINE_BITS, MAIN_MEM_BYTES> MainCache;
	typedef RegisterFile<WORD_SIZE, NUM_REGISTERS> RegFile;

	void Connect();
	void Update();

private:
	class PipelineStage : public Component
	{
	public:
		PipelineStage(std::mutex& mutex, std::condition_variable& cv, bool& ready)
		: mMutex(mutex)
		, mCV(cv)
		, mReady(ready)
		{}

		void ThreadedUpdate();
		virtual void Update() = 0;
		virtual void PostUpdate() = 0;

	private:
		std::condition_variable& mCV;
		std::mutex& mMutex;
		bool& mReady;
	};

	const Bundle<32>& PC();
	InsCache& InstructionMem();
	MainCache& MainMem();
	RegFile& Registers();

	bool InstructionFetchNop() { return InstructionMem().NeedStall().On(); }
	bool PipelineBubble() { return interlock.Bubble().On(); }
	bool PipelineFreeze() { return interlock.Freeze().On(); }
	bool Halt();

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

	friend class ProgramLoader;
	friend class Debugger;
	friend class PipelineStage;
};