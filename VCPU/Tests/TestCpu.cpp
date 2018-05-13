#include "TestCPU.h"
#include "CPU/CPU.h"
#include "MagicBundle.h"

unsigned int GetInstruction(unsigned int opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, unsigned int func)
{
	return (opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
}

bool TestCPU()
{
	CPU* pcpu = new CPU();
	CPU& cpu = *pcpu;

	MagicBundle<32> addr, ins;
	cpu.ConnectToLoader(addr, ins);

	ins.Write(GetInstruction(0, 0, 0, 0, 0, ALU_OPCODE::A_PLUS_ONE));
	addr.Write(0);
	cpu.LoadInstruction();
	ins.Write(GetInstruction(0, 0, 0, 1, 0, ALU_OPCODE::A));
	addr.Write(4);
	cpu.LoadInstruction();
	for (int i = 0; i < 25; i++)
	{
		ins.Write(GetInstruction(0, i, i+1, i+2, 0, ALU_OPCODE::A_PLUS_B));
		addr.Write(8 + 4*i);
		cpu.LoadInstruction();
	}

	cpu.Connect();
	
	while (true)
	{
		cpu.Update();
		if (cpu.cycles % 500 == 0)
			std::cout << cpu.cycles << std::endl;
	}
}