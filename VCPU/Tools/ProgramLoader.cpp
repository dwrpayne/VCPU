#include "ProgramLoader.h"
#include "Program.h"
#include "CPU/CPU.h"

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

void ProgramLoader::Load(const Program * program)
{
	int num_ins = program->Instructions().size();
	assert(num_ins < (memory.BYTES / 4));
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
		systembus.Update();
		while (!systembus.OutCtrl().Ack().On())
		{
			memory.DoOneUpdate();
			memory.WaitUntilDone();
			systembus.Update();
		}
		req.Set(false);
		systembus.Update();
		while (systembus.OutCtrl().Ack().On())
		{
			systembus.Update();
			memory.DoOneUpdate();
			memory.WaitUntilDone();
			systembus.Update();
		}
	}
	systembus.DisconnectAddr(addrBundle);
	systembus.DisconnectData(dataBundle);
	systembus.DisconnectCtrl(write, SystemBus::CtrlBit::Write);
	systembus.DisconnectCtrl(req, SystemBus::CtrlBit::Req);
}

