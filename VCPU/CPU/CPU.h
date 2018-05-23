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


class CPU : public Component
{
public:
	CPU();

	int cycles;

	typedef Memory<32, 128, 256> InsMemory;
	typedef Memory<32, 1024, 256> MainMemory;
	typedef Cache<32, 64, 256, InsMemory::BYTES> InsCache;
	typedef Cache<32, 256, 256, MainMemory::BYTES> MainCache;
	typedef RegisterFile<32, 32> RegFile;

	void Connect();
	void Update();

private:
	class PipelineStage : public Component
	{
	public:
		PipelineStage(CPU& c)
			: cpu(c)
		{}
	protected:
		CPU& cpu;
	};

	Register<32> PC();
	InsMemory& InstructionMem();
	MainMemory& MainMem();
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

	friend class ProgramLoader;
	friend class Debugger;
};