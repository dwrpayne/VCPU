#include "ProgramLoader.h"
#include "Program.h"
#include "CPU/Addresses.h" 

ProgramLoader::ProgramLoader(CPU & cpu)
	: systembus(cpu.GetSystemBus())
	, memory(cpu.InstructionMemory())
	, curAddr(0)
	, write(false)
{

	for (int i = 0, j = 0; i < cpu.CACHE_LINE_BITS; i += cpu.WORD_SIZE, ++j)
	{
		dataBundle.Connect(i, wordBundles[j]);
	}
	systembus.ConnectAddr(addrBundle);
	systembus.ConnectData(dataBundle);
	systembus.ConnectCtrl(write, SystemBus::CtrlBit::Write);
	systembus.ConnectCtrl(req, SystemBus::CtrlBit::Req);
}

ProgramLoader::~ProgramLoader()
{
	systembus.DisconnectAddr(addrBundle);
	systembus.DisconnectData(dataBundle);
	systembus.DisconnectCtrl(write, SystemBus::CtrlBit::Write);
	systembus.DisconnectCtrl(req, SystemBus::CtrlBit::Req);
}

void ProgramLoader::Load(const Program * program)
{
	LoadInstructions(program);
	LoadText(program);
}

void ProgramLoader::LoadInstructions(const Program * program)
{
	int num_ins = program->Instructions().size();
	assert(num_ins < (USER_TEXT_START - USER_CODE_START) / 4);
	write.Set(true);
	for (int i = 0; i < num_ins; i += 8)
	{
		addrBundle.Write(curAddr);
		for (int j = 0; j < 8 && (i+j < num_ins); j++)
		{
			unsigned int bin = program->Instructions()[i+j].mBinary;
			wordBundles[j].Write(bin);			
		}
		curAddr += 32;
		req.Set(true);
		while (!systembus.OutCtrl().Ack().On())
		{
			memory.DoOneUpdate();
			memory.WaitUntilDone();
		}
		req.Set(false);
		while (systembus.OutCtrl().Ack().On())
		{
			memory.DoOneUpdate();
			memory.WaitUntilDone();
		}
	}
}

void ProgramLoader::LoadText(const Program * program)
{
	curAddr = program->GetTextStartAddr();
	std::vector<unsigned char> textbytes = program->TextBytes();
	int missing = memory.CACHELINE_BYTES - (textbytes.size() % memory.CACHELINE_BYTES);
	for (int i = 0; i < missing; i++)
	{
		textbytes.push_back(0);
	}
	int num_bytes = textbytes.size();
	write.Set(true);
	for (int i = 0; i < num_bytes; i += memory.CACHELINE_BYTES)
	{
		addrBundle.Write(curAddr);
		for (int j = 0; j < memory.CACHELINE_BYTES && (i + j < num_bytes); j+=4)
		{
			unsigned char byte1 = textbytes[i + j];
			unsigned char byte2 = textbytes[i + j + 1];
			unsigned char byte3 = textbytes[i + j + 2];
			unsigned char byte4 = textbytes[i + j + 3];
			unsigned int byte = (byte1) | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);

			wordBundles[j/4].Write(byte);
		}
		curAddr += memory.CACHELINE_BYTES;
		req.Set(true);
		while (!systembus.OutCtrl().Ack().On())
		{
			memory.DoOneUpdate();
			memory.WaitUntilDone();
		}
		req.Set(false);
		while (systembus.OutCtrl().Ack().On())
		{
			memory.DoOneUpdate();
			memory.WaitUntilDone();
		}
	}
}

