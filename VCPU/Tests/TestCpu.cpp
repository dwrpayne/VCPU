#include "TestCPU.h"
#include "CPU/CPU.h"
#include "MagicBundle.h"


bool TestCPU()
{
	CPU* pcpu = new CPU();
	CPU& cpu = *pcpu;

	MagicBundle<32> addr, ins;
	cpu.ConnectToLoader(addr, ins);

	addr.Write(0);
	ins.Write(1);
	cpu.LoadInstruction();
	addr.Write(4);
	ins.Write(1);
	cpu.LoadInstruction();
	addr.Write(8);
	ins.Write(1);
	cpu.LoadInstruction();
	addr.Write(12);
	ins.Write(1);
	cpu.LoadInstruction();
	addr.Write(16);
	ins.Write(1);
	cpu.LoadInstruction();

	cpu.Connect();
	
	while (true)
	{
		cpu.Update();
	}
}