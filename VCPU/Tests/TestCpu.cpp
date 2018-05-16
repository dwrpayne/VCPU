#include <vector>
#include "TestCPU.h"
#include "TestHelpers.h"
#include "CPU/CPU.h"
#include "Tools/MagicBundle.h"
#include "CPU/ALUControl.h"
#include "Instructions.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"

bool TestOpcodeDecoder(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	MagicBundle<6> opcode;
	OpcodeDecoder test;
	test.Connect(opcode);

	std::cout << "Testing Branch Ops" << std::endl;
	for (auto op : { OP_BEQ, OP_BNE, OP_BLEZ, OP_BGTZ })
	{
		opcode.Write(op);
		test.Update();
		success &= TestState(i++, true, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadStore().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.IFormat().On(), verbosity);
		success &= TestState(i++, false, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, test.RegWrite().On(), verbosity);
	}

	std::cout << "Testing R-ALU Ops" << std::endl;
	for (auto op : { OP_ADD, OP_ADDU, OP_SUB, OP_SUBU, OP_AND, OP_OR, OP_XOR, OP_NOR })
	{
		opcode.Write(op);
		test.Update();
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadStore().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, true, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.IFormat().On(), verbosity);
		success &= TestState(i++, false, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
	}

	std::cout << "Testing I-ALU Ops" << std::endl;
	for (auto op : { OP_ADDI, OP_ADDIU, OP_SLTI, OP_SLTIU, OP_ANDI, OP_ORI, OP_XORI, OP_LUI })
	{
		opcode.Write(op);
		test.Update();
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, false, test.LoadStore().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, true, test.IFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
	}

	std::cout << "Testing Load Ops" << std::endl;
	for (auto op : { OP_LB, OP_LH, OP_LWL, OP_LW, OP_LBU, OP_LHU, OP_LWR })
	{
		opcode.Write(op);
		test.Update();
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, true, test.LoadStore().On(), verbosity);
		success &= TestState(i++, true, test.LoadOp().On(), verbosity);
		success &= TestState(i++, false, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.IFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, test.RegWrite().On(), verbosity);
	}

	std::cout << "Testing Store Ops" << std::endl;
	for (auto op : { OP_SB, OP_SH, OP_SWL, OP_SW, OP_SWR })
	{
		opcode.Write(op);
		test.Update();
		success &= TestState(i++, false, test.Branch().On(), verbosity);
		success &= TestState(i++, true, test.LoadStore().On(), verbosity);
		success &= TestState(i++, false, test.LoadOp().On(), verbosity);
		success &= TestState(i++, true, test.StoreOp().On(), verbosity);
		success &= TestState(i++, false, test.RFormat().On(), verbosity);
		success &= TestState(i++, false, test.IFormat().On(), verbosity);
		success &= TestState(i++, true, test.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, test.RegWrite().On(), verbosity);
	}
	return success;
}

bool TestALUControl(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	MagicBundle<6> opcode, func;
	OpcodeDecoder opdec;
	opdec.Connect(opcode);
	ALUControl test;
	test.Connect(opdec.LoadStore(), opdec.Branch(), opdec.IFormat(), opdec.RFormat(), opcode, func);

	std::cout << "Testing R-ALU Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, Function>>({
		{ A_PLUS_B, OP_ADD, F_ADD },
		{ A_PLUS_B, OP_ADDU, F_ADDU },
		{ A_MINUS_B, OP_SUB, F_SUB },
		{ A_MINUS_B, OP_SUBU, F_SUBU },
		{ A_AND_B, OP_AND, F_AND },
		{ A_OR_B, OP_OR, F_OR },
		{ A_XOR_B, OP_XOR, F_XOR },
		{ A_NOR_B, OP_NOR, F_NOR } }))
	{
		opcode.Write(op);
		func.Write(f);
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
	}

	std::cout << "Testing I-ALU Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_ADDI, 45 },
		{ A_PLUS_B, OP_ADDIU, 45 },
		//{ A_MINUS_B, OP_SLTI, 45 },
		//{ A_MINUS_B, OP_SLTIU, 45 },
		{ A_AND_B, OP_ANDI, 45 },
		{ A_OR_B, OP_ORI, 45 },
		{ A_XOR_B, OP_XORI, 45 },
		/*{ A_NOR_B, OP_LUI, 45 }*/ }))
	{
		opcode.Write(op);
		func.Write(f);
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
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
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
	}

	std::cout << "Testing Load/Store Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_LB, 45 },
		{ A_PLUS_B, OP_LH, 45 },
		{ A_PLUS_B, OP_LWL, 45 },
		{ A_PLUS_B, OP_LW, 45 },
		{ A_PLUS_B, OP_LBU, 45 },
		{ A_PLUS_B, OP_LHU, 45 },
		{ A_PLUS_B, OP_LWR, 45 },
		{ A_PLUS_B, OP_SB, 45 },
		{ A_PLUS_B, OP_SH, 45 },
		{ A_PLUS_B, OP_SWL, 45 },
		{ A_PLUS_B, OP_SW, 45 },
		{ A_PLUS_B, OP_SWR, 45 } }))
	{
		opcode.Write(op);
		func.Write(f);
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
	}
	return success;
}

bool TestCPU(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	CPU* pcpu = new CPU();
	Assembler assembler("testops.vasm");
	Debugger debugger(assembler.GetProgram());

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
	debugger.Step();															// and 1 2 5 0
	success &= TestState(i++, 20, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 262, debugger.GetRegisterVal(5), verbosity);
	debugger.Step();															// or 1 2 6 0
	success &= TestState(i++, 24, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(6), verbosity);
	debugger.Step();																// xor 1 2 7 0
	success &= TestState(i++, 28, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 3801, debugger.GetRegisterVal(7), verbosity);
	debugger.Step();																// nor 1 2 8 0
	success &= TestState(i++, 32, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, -4064, debugger.GetRegisterVal(8), verbosity);
	debugger.Step();																// addi 3 13 1234
	success &= TestState(i++, 36, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 5559, debugger.GetRegisterVal(13), verbosity);
	debugger.Step();		 														// andi 5 15 1234
	success &= TestState(i++, 40, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 2, debugger.GetRegisterVal(15), verbosity);
	debugger.Step();																// ori 6 16 1234
	success &= TestState(i++, 44, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(16), verbosity);
	debugger.Step();																// xori 7 17 1234
	success &= TestState(i++, 48, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 2571, debugger.GetRegisterVal(17), verbosity);
	debugger.Step();																// ori 0 22 4325
	success &= TestState(i++, 52, debugger.GetNextPCAddr(), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(22), verbosity);
	debugger.Step();																// beq 3 22 4
	success &= TestState(i++, 60, debugger.GetNextPCAddr(), verbosity);
	debugger.Step();																// bne 3 22 -38
	success &= TestState(i++, 64, debugger.GetNextPCAddr(), verbosity);
	debugger.Step();																// and 0 0 0 0

	return success;
}


bool RunCPUTests()
{
	bool success = true;
	RUN_TEST(TestOpcodeDecoder, FAIL_ONLY);
	RUN_TEST(TestALUControl, FAIL_ONLY);
	RUN_TEST(TestCPU, FAIL_ONLY);

	return success;
}
