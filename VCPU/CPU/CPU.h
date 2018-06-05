#pragma once

#include "Component.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "BranchDetector.h"
#include "PipelineBuffers.h"
#include "Cache.h"
#include "HazardUnit.h"


class CPU : public Component
{
public:
	CPU();

	int cycles;
	static const int WORD_SIZE = 32;
	static const int CACHE_LINE_BITS = 256;
	static const int NUM_REGISTERS = 32;

	static const int INS_CACHE_BYTES = 64;
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
		PipelineStage()	{}
		virtual void PostUpdate() = 0;
	};

	Register<32> PC();
	InsCache& InstructionMem();
	MainCache& MainMem();
	RegFile& Registers();

private:
	class Stage1;
	class Stage2;
	class Stage3;
	class Stage4;

	Stage1* stage1;
	Stage2* stage2;
	Stage3* stage3;
	Stage4* stage4;

	HazardUnit hazard;

	friend class ProgramLoader;
	friend class Debugger;
};