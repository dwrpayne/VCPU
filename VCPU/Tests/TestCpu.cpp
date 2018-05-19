#include <vector>
#include "TestCPU.h"
#include "TestHelpers.h"
#include "CPU/Cache.h"
#include "CPU/CPU.h"
#include "Tools/MagicBundle.h"
#include "Instructions.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"

bool TestOpcodeDecoder(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	MagicBundle<6> opcode, func;
	OpcodeDecoder test;
	test.Connect(opcode, func);

	std::cout << "Testing R-ALU Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, Function>>({
		{ A_PLUS_B, OP_ADD, F_ADD },
		{ A_PLUS_B, OP_ADDU, F_ADDU },
		{ A_MINUS_B, OP_SUB, F_SUB },
		{ A_MINUS_B, OP_SUBU, F_SUBU },
		{ A_AND_B, OP_AND, F_AND },
		{ A_OR_B, OP_OR, F_OR },
		{ A_XOR_B, OP_XOR, F_XOR },
		{ A_NOR_B, OP_NOR, F_NOR },
		{ A_MINUS_B, OP_SLT, F_SLT },
		{ A_MINUS_B, OP_SLTU, F_SLTU } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, true, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
		success &= TestState(i++, (f == F_SLT || f == F_SLTU), test.SltOp().On(), verbosity);

	}

	std::cout << "Testing I-ALU Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_ADDI, 45 },
		{ A_PLUS_B, OP_ADDIU, 45 },
		{ A_MINUS_B, OP_SLTI, 45 },
		{ A_MINUS_B, OP_SLTIU, 45 },
		{ A_AND_B, OP_ANDI, 45 },
		{ A_OR_B, OP_ORI, 45 },
		{ A_XOR_B, OP_XORI, 45 },
		/*{ A_NOR_B, OP_LUI, 45 }*/ }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
		success &= TestState(i++, (op == OP_SLTI || op == OP_SLTIU), test.SltOp().On(), verbosity);
	}

	std::cout << "Testing Branch Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_MINUS_B, OP_BEQ, 45 },
		{ A_MINUS_B, OP_BNE, 45 },
		{ A_MINUS_B, OP_BLEZ, 45 },
		{ A_MINUS_B, OP_BGTZ, 45 } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, true, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, test.RegWrite().On(), verbosity);
		success &= TestState(i++, false, test.SltOp().On(), verbosity);
	}

	std::cout << "Testing Load Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_LB, 45 },
		{ A_PLUS_B, OP_LH, 45 },
		{ A_PLUS_B, OP_LWL, 45 },
		{ A_PLUS_B, OP_LW, 45 },
		{ A_PLUS_B, OP_LBU, 45 },
		{ A_PLUS_B, OP_LHU, 45 },
		{ A_PLUS_B, OP_LWR, 45 } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, true, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
		success &= TestState(i++, false, test.SltOp().On(), verbosity);
	}

	std::cout << "Testing Store Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_SB, 45 },
		{ A_PLUS_B, OP_SH, 45 },
		{ A_PLUS_B, OP_SWL, 45 },
		{ A_PLUS_B, OP_SW, 45 },
		{ A_PLUS_B, OP_SWR, 45 } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, true, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, test.RegWrite().On(), verbosity);
		success &= TestState(i++, false, test.SltOp().On(), verbosity);
	}
	return success;
}

bool TestCache(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Memory<256, 256>* pMainMem = new Memory<256, 256>();

	MagicBundle<256> data256;
	std::array<MagicBundle<32>, 8> data;
	for (int i = 0; i < 8; i++)
	{
		data256.Connect(i * 32, data[i]);
	}
	Wire load(true);
	MagicBundle<13> addr;
	pMainMem->Connect(addr, data256, load);


	for (int i = 0; i < 8; i++)
	{
		data[i].Write(100000000 + 1111111 * i);
	}
	addr.Write(8);
	pMainMem->Update();

	for (int i = 0; i < 8; i++)
	{
		data[i].Write(200000000 + 1111111 * i);
	}
	addr.Write(68);
	pMainMem->Update();
	
	Cache* pCache = new Cache();
	Cache& test = *pCache;
	test.Connect(addr, pMainMem->Out(), Wire::OFF);
	
	addr.Write(8);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	pMainMem->Update();
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	addr.Write(4);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	addr.Write(20);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	addr.Write(16);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);

	addr.Write(64);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	pMainMem->Update();
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	addr.Write(70);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);

	return success;
}

bool TestCPU(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	CPU* pcpu = new CPU();
	Debugger debugger("testops.vasm");
	debugger.SetInstructionPrint(true);

	success &= TestState(i++, 0, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(0), verbosity);

	debugger.Step();																// addi 0 1 1887
	success &= TestState(i++, 4, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 1887, debugger.GetRegisterVal(1), verbosity);

	debugger.Step();																// addi 0 2 2438
	success &= TestState(i++, 8, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 2438, debugger.GetRegisterVal(2), verbosity);

	debugger.Step();		 														// add 1 2 3 0 
	success &= TestState(i++, 12, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(3), verbosity);

	debugger.Step();																// sub 1 2 4 0 
	success &= TestState(i++, 16, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, -551, debugger.GetRegisterVal(4), verbosity);

	debugger.Step();																// and 1 2 5 0
	success &= TestState(i++, 20, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 262, debugger.GetRegisterVal(5), verbosity);

	debugger.Step();																// or 1 2 6 0
	success &= TestState(i++, 24, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(6), verbosity);

	debugger.Step();																// xor 1 2 7 0
	success &= TestState(i++, 28, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 3801, debugger.GetRegisterVal(7), verbosity);

	debugger.Step();																// nor 1 2 8 0
	success &= TestState(i++, 32, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, -4064, debugger.GetRegisterVal(8), verbosity);

	debugger.Step();																// slt 1 2 9 0
	success &= TestState(i++, 36, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 1, debugger.GetRegisterVal(9), verbosity);

	debugger.Step();																// ori 0 10 11111
	success &= TestState(i++, 40, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 11111, debugger.GetRegisterVal(10), verbosity);

	debugger.Step();																// slt 3 2 10 0
	success &= TestState(i++, 44, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(10), verbosity);

	debugger.Step();																// addi 3 13 1234
	success &= TestState(i++, 48, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 5559, debugger.GetRegisterVal(13), verbosity);

	debugger.Step();		 														// andi 5 15 1234
	success &= TestState(i++, 52, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 2, debugger.GetRegisterVal(15), verbosity);

	debugger.Step();																// ori 6 16 1234
	success &= TestState(i++, 56, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(16), verbosity);

	debugger.Step();																// xori 7 17 1234
	success &= TestState(i++, 60, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 2571, debugger.GetRegisterVal(17), verbosity);

	debugger.Step();																// slti 6 19 4064
	success &= TestState(i++, 64, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 1, debugger.GetRegisterVal(19), verbosity);

	debugger.Step();																// slti 6 19 4063
	success &= TestState(i++, 68, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(19), verbosity);

	debugger.Step();																// ori 0 22 4325
	success &= TestState(i++, 72, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(22), verbosity);

	debugger.Step();																// sw 15 1 14  (store R1 into R15(14). R15 is 2 so mem 16
	success &= TestState(i++, 76, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 1887, debugger.GetMemoryVal(16), verbosity);

	debugger.Step();																// lw 5 11 -246 (load R5(-246) into R11.
	success &= TestState(i++, 80, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 1887, debugger.GetRegisterVal(11), verbosity);

	debugger.Step();																// beq 3 22 8 (should take)
	success &= TestState(i++, 92, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// beq 3 20 4 (should not take)
	success &= TestState(i++, 96, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// and 0 0 0
	success &= TestState(i++, 100, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// bne 3 22 -38 (should not take)
	success &= TestState(i++, 104, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// and 0 0 0
	success &= TestState(i++, 108, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// bne 3 20 12 (should take)
	success &= TestState(i++, 124, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// blez 8 0 8 (should take)
	success &= TestState(i++, 136, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// blez 19 0 4 (should take)
	success &= TestState(i++, 144, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// blez 16 0 4 (should not take)
	success &= TestState(i++, 148, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// and 0 0 0
	success &= TestState(i++, 152, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// bgtz 8 0 4 (should not take)
	success &= TestState(i++, 156, debugger.GetNextPCAddr(), verbosity);
	debugger.Step();																// and 0 0 0
	success &= TestState(i++, 160, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// bgtz 19 0 4 (should not take)
	success &= TestState(i++, 164, debugger.GetNextPCAddr(), verbosity);
	debugger.Step();																// and 0 0 0
	success &= TestState(i++, 168, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// bgtz 16 0 20 (should take)
	success &= TestState(i++, 192, debugger.GetNextPCAddr(), verbosity);

	debugger.Step();																// beq 1 11 -196 (should take back to start
	success &= TestState(i++, 0, debugger.GetNextPCAddr(), verbosity);


	return success;
}


bool RunCPUTests()
{
	bool success = true;
	RUN_TEST(TestOpcodeDecoder, FAIL_ONLY);
	RUN_TEST(TestCache, FAIL_ONLY);
	RUN_TEST(TestCPU, FAIL_ONLY);

	return success;
}
