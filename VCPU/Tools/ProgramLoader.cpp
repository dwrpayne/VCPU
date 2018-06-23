#include "ProgramLoader.h"
#include "Program.h"
#include "CPU/CPU.h"

ProgramLoader::ProgramLoader(CPU & cpu)
	: systembus(cpu.GetSystemBus())
	, curAddr(0)
	, availBytes(cpu.InstructionMemory().BYTES)
	, write(false)
{
	systembus.ConnectAddr(addrBundle);

	Bundle<cpu.CACHE_LINE_BITS> data_bundle;
	for (int i = 0, j = 0; i < cpu.CACHE_LINE_BITS; i += cpu.WORD_SIZE, ++j)
	{
		data_bundle.Connect(i, wordBundles[j]);
	}
	systembus.ConnectData(data_bundle);
	systembus.ConnectCtrl(write, SystemBus::CtrlBit::Write);
}

void ProgramLoader::Load(const Program * program)
{
	int num_ins = program->Instructions().size();
	assert(num_ins < (availBytes / 4));
	for (int i = 0; i < num_ins; i += 8)
	{
		addrBundle.Write(curAddr);
		for (int j = 0; j < 8 && (i+j < num_ins); j++)
		{
			unsigned int bin = program->Instructions()[i+j].mBinary;
			wordBundles[j].Write(bin);			
		}
		curAddr += 32;
		write.Set(true);
		while (!systembus.OutCtrl().Ack().On())
		{
			systembus.Update();
		}
		write.Set(false);
		while (systembus.OutCtrl().Ack().On())
		{
			systembus.Update();
		}
	}
}

