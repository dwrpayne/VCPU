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
		{ A_MINUS_B, OP_SLTU, F_SLTU },
		{ A_SHLL, OP_SLL, F_SLL },
		{ A_SHLL, OP_SLLV, F_SLLV },
		{ A_SHRL, OP_SRL, F_SRL },
		{ A_SHRL, OP_SRLV, F_SRLV },
		{ A_SHRA, OP_SRA, F_SRA },
		{ A_SHRA, OP_SRAV, F_SRAV } }))
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
		success &= TestState(i++, (f == F_SLL || f == F_SRL || f == F_SRA || f == F_SLLV || f == F_SRLV || f == F_SRAV), test.ShiftOp().On(), verbosity);
		success &= TestState(i++, (f == F_SLL || f == F_SRL || f == F_SRA), test.ShiftAmtOp().On(), verbosity);
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

	Cache<32, 512, 256, 2048>* pCache = new Cache<32, 512, 256, 2048>();
	Cache<32, 512, 256, 2048>& test = *pCache;

	MagicBundle<32> data;
	Wire write(true);
	Wire read(false);
	MagicBundle<11> addr;
	test.Connect(addr, data, write, read);

	for (int a = 0; a < 8; a++)
	{
		addr.Write(0 + 4 * a);
		data.Write(100000000 + 1111111 * a);
		test.Update();
	}

	for (int a = 0; a < 8; ++a)
	{
		addr.Write(64 + 4*a );
		data.Write(200000000 + 1111111 * a);
		test.Update();
	}
	write.Set(false);
	read.Set(true);
		
	addr.Write(0);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 100000000, test.Out().Read(), verbosity);
	for (int a = 0; a < 8; a++)
	{
		addr.Write(4*a);
		test.Update();
		success &= TestState(i++, true, test.CacheHit().On(), verbosity);
		success &= TestState(i++, 100000000 + 1111111 * a, test.Out().Read(), verbosity);
	}
	addr.Write(88);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	test.Update();
	for (int a = 0; a < 8; a++)
	{
		addr.Write(64 + 4*a);
		test.Update();
		success &= TestState(i++, true, test.CacheHit().On(), verbosity);
		success &= TestState(i++, 200000000 + 1111111 * a, test.Out().Read(), verbosity);
	}

	data.Write(4444);
	addr.Write(4);
	write.Set(true);
	read.Set(false);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 4444, test.Out().Read(), verbosity);
	test.Update();

	data.Write(525252);
	addr.Write(52);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	test.Update();

	data.Write(123);
	write.Set(false);
	read.Set(true);
	addr.Write(32);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);
	addr.Write(52);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 525252, test.Out().Read(), verbosity);

	return success;
}

bool TestCPU(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testops.vasm", dverb);
	debugger.Start();	
	success &= TestState(i++, 1887, debugger.GetRegisterVal(1), verbosity);
	success &= TestState(i++, 2438, debugger.GetRegisterVal(2), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(3), verbosity);
	success &= TestState(i++, -551, debugger.GetRegisterVal(4), verbosity);
	success &= TestState(i++, 262, debugger.GetRegisterVal(5), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(6), verbosity);
	success &= TestState(i++, 3801, debugger.GetRegisterVal(7), verbosity);
	success &= TestState(i++, -4064, debugger.GetRegisterVal(8), verbosity);
	success &= TestState(i++, 1, debugger.GetRegisterVal(9), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(10), verbosity);
	success &= TestState(i++, 5559, debugger.GetRegisterVal(13), verbosity);
	success &= TestState(i++, 2, debugger.GetRegisterVal(15), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(16), verbosity);
	success &= TestState(i++, 2571, debugger.GetRegisterVal(17), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(19), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(22), verbosity);
	success &= TestState(i++, 1887, debugger.GetMemoryVal(16), verbosity);
	success &= TestState(i++, 1887, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(20), verbosity);
	success &= TestState(i++, 22691, debugger.GetRegisterVal(23), verbosity);
	success &= TestState(i++, 5, debugger.GetRegisterVal(24), verbosity);
	success &= TestState(i++, 5808896, debugger.GetRegisterVal(25), verbosity);
	success &= TestState(i++, 726112, debugger.GetRegisterVal(26), verbosity);
	success &= TestState(i++, 2836, debugger.GetRegisterVal(27), verbosity);
	success &= TestState(i++, 709, debugger.GetRegisterVal(28), verbosity);
	success &= TestState(i++, -35, debugger.GetRegisterVal(29), verbosity);
	success &= TestState(i++, -18, debugger.GetRegisterVal(30), verbosity);
	return success;
}


bool TestCPUBranch(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testbranch.vasm", dverb);
	debugger.Start();

	return success;
}

bool TestCPUPipelineHazards(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testhazards.vasm", dverb);
	debugger.Start();
	success &= TestState(i++, 1234, debugger.GetRegisterVal(1), verbosity);
	success &= TestState(i++, 1357, debugger.GetRegisterVal(2), verbosity);
	success &= TestState(i++, 1603, debugger.GetRegisterVal(3), verbosity);
	success &= TestState(i++, 2591, debugger.GetRegisterVal(4), verbosity);
	success &= TestState(i++, 2837, debugger.GetRegisterVal(5), verbosity);
	success &= TestState(i++, 5428, debugger.GetRegisterVal(6), verbosity);
	success &= TestState(i++, -2591, debugger.GetRegisterVal(7), verbosity);
	success &= TestState(i++, -5182, debugger.GetRegisterVal(8), verbosity);
	success &= TestState(i++, 1234, debugger.GetRegisterVal(10), verbosity);
	success &= TestState(i++, 1238, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(12), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(13), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(14), verbosity);
	success &= TestState(i++, 366, debugger.GetRegisterVal(20), verbosity);
	success &= TestState(i++, 123, debugger.GetRegisterVal(21), verbosity);
	success &= TestState(i++, 1234, debugger.GetMemoryVal(4), verbosity);
	success &= TestState(i++, 156, debugger.GetNextPCAddr(), verbosity);

	return success;
}

bool RunCPUTests()
{
	bool success = true;
	RUN_TEST(TestOpcodeDecoder, FAIL_ONLY);
	//RUN_TEST(TestCache, FAIL_ONLY);
	RUN_TEST2(TestCPU, FAIL_ONLY, Debugger::TIMING);
	RUN_TEST2(TestCPUPipelineHazards, FAIL_ONLY, Debugger::TIMING);
	RUN_TEST2(TestCPUBranch, VERBOSE, Debugger::TIMING);

	return success;
}
