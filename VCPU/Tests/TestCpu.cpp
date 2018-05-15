#include <vector>
#include "TestCPU.h"
#include "TestHelpers.h"
#include "CPU/CPU.h"
#include "MagicBundle.h"
#include "CPU/ALUControl.h"


enum Opcode : uint8_t
{
	OP_SLL = 0,
	OP_SRL = 0,
	OP_SRA = 0,
	OP_SLLV = 0,
	OP_SRLV = 0,
	OP_SRAV = 0,

	OP_JR = 0,
	OP_JALR = 0,

	OP_MFHI = 0,
	OP_MTHI = 0,
	OP_MFLO = 0,
	OP_MTLO = 0,
	OP_MULT = 0,
	OP_MULTU = 0,
	OP_DIV = 0,
	OP_DIVU = 0,

	OP_ADD = 0,
	OP_ADDU = 0,
	OP_SUB = 0,
	OP_SUBU = 0,
	OP_AND = 0,
	OP_OR = 0,
	OP_XOR = 0,
	OP_NOR = 0,
	OP_SLT = 0,
	OP_SLTU = 0,

	OP_BLTZ = 1,
	OP_BGEZ = 1,
	OP_BLTZAL = 1,
	OP_BGEZAL = 1,
	OP_J = 2,
	OP_JAL = 3,
	OP_BEQ = 4,
	OP_BNE = 5,
	OP_BLEZ = 6,
	OP_BGTZ = 7,

	OP_ADDI = 8,
	OP_ADDIU = 9,
	OP_SLTI = 10,
	OP_SLTIU = 11,
	OP_ANDI = 12,
	OP_ORI = 13,
	OP_XORI = 14,
	OP_LUI = 15,

	OP_LB = 32,
	OP_LH = 33,
	OP_LWL = 34,
	OP_LW = 35,
	OP_LBU = 36,
	OP_LHU = 37,
	OP_LWR = 38,
	OP_SB = 40,
	OP_SH = 41,
	OP_SWL = 42, 
	OP_SW = 43,
	OP_SWR = 46,
};


enum Function : uint8_t
{
	F_SLL = 0,
	F_SRL = 2,
	F_SRA = 3,
	F_SLLV = 4,
	F_SRLV = 6,
	F_SRAV = 7,

	F_JR = 8,
	F_JALR = 9,

	F_MFHI = 16,
	F_MTHI = 17,
	F_MFLO = 18,
	F_MTLO = 19,
	F_MULT = 24,
	F_MULTU = 25,
	F_DIV = 26,
	F_DIVU = 27,

	F_ADD = 32,
	F_ADDU = 33,
	F_SUB = 34,
	F_SUBU = 35,
	F_AND = 36,
	F_OR = 37,
	F_XOR = 38,
	F_NOR = 39,
	F_SLT = 42,
	F_SLTU = 43,

	F_BLTZ = 0,
	F_BGEZ = 0,
	F_BLTZAL = 0,
	F_BGEZAL = 0,
	F_J = 0,
	F_JAL = 0,
	F_BEQ = 0,
	F_BNE = 0,
	F_BLEZ = 0,
	F_BGTZ = 0,
	F_ADDI = 0,
	F_ADDIU = 0,
	F_SLTI = 0,
	F_SLTIU = 0,
	F_ANDI = 0,
	F_ORI = 0,
	F_XORI = 0,
	F_LUI = 0,
	F_LB = 0,
	F_LH = 0,
	F_LWL = 0,
	F_LW = 0,
	F_LBU = 0,
	F_LHU = 0,
	F_LWR = 0,
	F_SB = 0,
	F_SH = 0,
	F_SWL = 0,
	F_SW = 0,
	F_SWR = 0,
};

unsigned int GetInstruction(Opcode opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, Function func)
{
	return (opcode << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
}

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

	std::cout << "Testing Branch Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, Function>>({
		{ A_MINUS_B, OP_BEQ, F_BEQ },
		{ A_MINUS_B, OP_BNE, F_BNE },
		{ A_MINUS_B, OP_BLEZ, F_BLEZ },
		{ A_MINUS_B, OP_BGTZ, F_BGTZ } }))
	{
		opcode.Write(op);
		func.Write(f);
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
	}

	std::cout << "Testing Load/Store Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, Function>>({
		{ A_PLUS_B, OP_LB, F_LB },
		{ A_PLUS_B, OP_LH, F_LH },
		{ A_PLUS_B, OP_LWL, F_LWL },
		{ A_PLUS_B, OP_LW, F_LW },
		{ A_PLUS_B, OP_LBU, F_LBU },
		{ A_PLUS_B, OP_LHU, F_LHU },
		{ A_PLUS_B, OP_LWR, F_LWR },
		{ A_PLUS_B, OP_SB, F_SB },
		{ A_PLUS_B, OP_SH, F_SH },
		{ A_PLUS_B, OP_SWL, F_SWL },
		{ A_PLUS_B, OP_SW, F_SW },
		{ A_PLUS_B, OP_SWR, F_SWR } }))
	{
		opcode.Write(op);
		func.Write(f);
		opdec.Update();
		test.Update();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
	}
	return success;
}

bool TestCPU()
{
	CPU* pcpu = new CPU();
	CPU& cpu = *pcpu;

	MagicBundle<32> addr, ins;
	cpu.ConnectToLoader(addr, ins);

	//ins.Write(GetInstruction(0, 0, 0, 0, 0, ALU_OPCODE::A_PLUS_ONE));
	//addr.Write(0);
	//cpu.LoadInstruction();
	//ins.Write(GetInstruction(0, 0, 0, 1, 0, ALU_OPCODE::A));
	//addr.Write(4);
	//cpu.LoadInstruction();
	//for (int i = 0; i < 25; i++)
	//{
	//	ins.Write(GetInstruction(0, i, i+1, i+2, 0, ALU_OPCODE::A_PLUS_B));
	//	addr.Write(8 + 4*i);
	//	cpu.LoadInstruction();
	//}

	//cpu.Connect();
	//
	//while (true)
	//{
	//	cpu.Update();
	//	if (cpu.cycles % 500 == 0)
	//		std::cout << cpu.cycles << std::endl;
	//}
	return true;
}

bool RunCPUTests()
{
	bool success = true;
	RUN_TEST(TestOpcodeDecoder, FAIL_ONLY);
	RUN_TEST(TestALUControl, FAIL_ONLY);

	return success;
}
