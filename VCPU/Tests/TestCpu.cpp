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
		{ A_PLUS_B, OP_MFHI, F_MFHI },
		{ A_MINUS_B, OP_MFLO, F_MFLO },
		{ A_PLUS_B, OP_MULT, F_MULT },
		{ A_PLUS_B, OP_MULTU, F_MULTU },
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
		const auto& out = test.OutBundle();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, out.Branch().On(), verbosity);
		success &= TestState(i++, false, out.LoadOp().On(), verbosity);
		success &= TestState(i++, false, out.StoreOp().On(), verbosity);
		success &= TestState(i++, true, out.RFormat().On(), verbosity);
		success &= TestState(i++, false, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, out.RegWrite().On(), verbosity);
		success &= TestState(i++, (f == F_SLT || f == F_SLTU), out.SltOp().On(), verbosity);
		success &= TestState(i++, (f == F_SLL || f == F_SRL || f == F_SRA || f == F_SLLV || f == F_SRLV || f == F_SRAV), out.ShiftOp().On(), verbosity);
		success &= TestState(i++, (f == F_SLL || f == F_SRL || f == F_SRA), out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, false, out.LoadUpperImm().On(), verbosity);
		success &= TestState(i++, (f == F_MULT || f == F_MULTU), out.MultOp().On(), verbosity);
		success &= TestState(i++, (f == F_MFLO || f == F_MFHI), out.MultMoveReg()[1].On(), verbosity);
		if (f == F_MFLO || f == F_MFHI)
		{
			success &= TestState(i++, f == F_MFLO, out.MultMoveReg()[0].On(), verbosity);
		}

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
		{ A_OR_B, OP_LUI, 45 } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		const auto& out = test.OutBundle();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, out.Branch().On(), verbosity);
		success &= TestState(i++, false, out.LoadOp().On(), verbosity);
		success &= TestState(i++, false, out.StoreOp().On(), verbosity);
		success &= TestState(i++, false, out.RFormat().On(), verbosity);
		success &= TestState(i++, true, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, out.RegWrite().On(), verbosity);
		success &= TestState(i++, (op == OP_SLTI || op == OP_SLTIU), out.SltOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpLink().On(), verbosity);
		success &= TestState(i++, false, out.JumpReg().On(), verbosity);
		success &= TestState(i++, (op == OP_LUI), out.LoadUpperImm().On(), verbosity);
		success &= TestState(i++, (op == OP_ADDI || op == OP_ADDIU || op == OP_SLTI || op == OP_SLTIU || op == OP_LUI), out.MathOp().On(), verbosity);

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
		const auto& out = test.OutBundle();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, true, out.Branch().On(), verbosity);
		success &= TestState(i++, false, out.LoadOp().On(), verbosity);
		success &= TestState(i++, false, out.StoreOp().On(), verbosity);
		success &= TestState(i++, false, out.RFormat().On(), verbosity);
		success &= TestState(i++, false, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, out.RegWrite().On(), verbosity);
		success &= TestState(i++, false, out.SltOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpLink().On(), verbosity);
		success &= TestState(i++, false, out.JumpReg().On(), verbosity);
		success &= TestState(i++, false, out.LoadUpperImm().On(), verbosity);
	}

	std::cout << "Testing Jump Ops" << std::endl;
	for (auto[op, f] : std::vector<std::tuple<Opcode, unsigned int>>({
		{ OP_J, 0 },
		{ OP_JAL, 40 },
		{ OP_JR, F_JR },
		{ OP_JALR, F_JALR } }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		const auto& out = test.OutBundle();
		success &= TestState(i++, false, out.Branch().On(), verbosity);
		success &= TestState(i++, false, out.LoadOp().On(), verbosity);
		success &= TestState(i++, false, out.StoreOp().On(), verbosity);
		success &= TestState(i++, (f == F_JR || f == F_JALR), out.RFormat().On(), verbosity);
		success &= TestState(i++, false, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, (op != OP_J), out.RegWrite().On(), verbosity);
		success &= TestState(i++, false, out.SltOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, true, out.JumpOp().On(), verbosity);
		success &= TestState(i++, (op == OP_JAL || f == F_JALR), out.JumpLink().On(), verbosity);
		success &= TestState(i++, (f == F_JR || f == F_JALR), out.JumpReg().On(), verbosity);
		success &= TestState(i++, false, out.LoadUpperImm().On(), verbosity);
	}

	std::cout << "Testing Load Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_LB, 45 },
		{ A_PLUS_B, OP_LH, 45 },
		//{ A_PLUS_B, OP_LWL, 45 },
		{ A_PLUS_B, OP_LW, 45 },
		{ A_PLUS_B, OP_LBU, 45 },
		{ A_PLUS_B, OP_LHU, 45 },
		/*{ A_PLUS_B, OP_LWR, 45 }*/ }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		const auto& out = test.OutBundle();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, out.Branch().On(), verbosity);
		success &= TestState(i++, true, out.LoadOp().On(), verbosity);
		success &= TestState(i++, false, out.StoreOp().On(), verbosity);
		success &= TestState(i++, false, out.RFormat().On(), verbosity);
		success &= TestState(i++, true, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, true, out.RegWrite().On(), verbosity);
		success &= TestState(i++, false, out.SltOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpLink().On(), verbosity);
		success &= TestState(i++, false, out.JumpReg().On(), verbosity);
		success &= TestState(i++, false, out.LoadUpperImm().On(), verbosity);
		success &= TestState(i++, (op != OP_LBU && op != OP_LHU), out.LoadSigned().On(), verbosity);
		success &= TestState(i++, (op == OP_LB || op == OP_LBU), out.MemOpByte().On(), verbosity);
		success &= TestState(i++, (op == OP_LH || op == OP_LHU), out.MemOpHalfWord().On(), verbosity);
	}

	std::cout << "Testing Store Ops" << std::endl;
	for (auto[alu, op, f] : std::vector<std::tuple<ALU_OPCODE, Opcode, unsigned int>>({
		{ A_PLUS_B, OP_SB, 45 },
		{ A_PLUS_B, OP_SH, 45 },
		//{ A_PLUS_B, OP_SWL, 45 },
		{ A_PLUS_B, OP_SW, 45 },
		/*{ A_PLUS_B, OP_SWR, 45 }*/ }))
	{
		opcode.Write(op);
		func.Write(f);
		test.Update();
		const auto& out = test.OutBundle();
		success &= TestState(i++, (unsigned int)alu, test.AluControl().UnsignedRead(), verbosity);
		success &= TestState(i++, false, out.Branch().On(), verbosity);
		success &= TestState(i++, false, out.LoadOp().On(), verbosity);
		success &= TestState(i++, true, out.StoreOp().On(), verbosity);
		success &= TestState(i++, false, out.RFormat().On(), verbosity);
		success &= TestState(i++, true, out.AluBFromImm().On(), verbosity);
		success &= TestState(i++, false, out.RegWrite().On(), verbosity);
		success &= TestState(i++, false, out.SltOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftOp().On(), verbosity);
		success &= TestState(i++, false, out.ShiftAmtOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpOp().On(), verbosity);
		success &= TestState(i++, false, out.JumpLink().On(), verbosity);
		success &= TestState(i++, false, out.JumpReg().On(), verbosity);
		success &= TestState(i++, false, out.LoadUpperImm().On(), verbosity);
		success &= TestState(i++, (op == OP_SB), out.MemOpByte().On(), verbosity);
		success &= TestState(i++, (op == OP_SH), out.MemOpHalfWord().On(), verbosity);

	}
	return success;
}

bool TestByteMask(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	ByteMask test;
	MagicBundle<2> index;
	Wire wb, wh, ww;

	test.Connect(index, wb, wh, ww);
	for (unsigned int a = 0; a < 4; a++)
	{
		index.Write(a);
		wb.Set(false);
		wh.Set(false);
		ww.Set(false);
		test.Update();
		success &= TestState(i++, 0U, test.Mask().UnsignedRead(), verbosity);
		wb.Set(false);
		wh.Set(false);
		ww.Set(true);
		test.Update();
		success &= TestState(i++, 0xffffffffU, test.Mask().UnsignedRead(), verbosity);
		wb.Set(false);
		wh.Set(true);
		ww.Set(false);
		test.Update();
		success &= TestState(i++, 0U, test.Mask().UnsignedRead(), verbosity);
		wb.Set(false);
		wh.Set(true);
		ww.Set(true);
		test.Update();
		success &= TestState(i++, 0xffffU << ((int)index[1].On() * 16), test.Mask().UnsignedRead(), verbosity);
		wb.Set(true);
		wh.Set(false);
		ww.Set(false);
		test.Update();
		success &= TestState(i++, 0U, test.Mask().UnsignedRead(), verbosity);
		wb.Set(true);
		wh.Set(false);
		ww.Set(true);
		test.Update();
		success &= TestState(i++, 0xffU << (index.Read() * 8), test.Mask().UnsignedRead(), verbosity);
	}

	return success;
}

bool TestCacheLineMasker(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	CacheLineMasker<64> test;
	MagicBundle<2> index;
	MagicBundle<32> dataword;
	MagicBundle<64> dataline;
	MagicBundle<1> offset;
	Wire wb, wh, ww;

	test.Connect(index, offset, dataword, dataline, wb, wh, ww);
	ww.Set(false);
	wh.Set(false);
	wb.Set(false);
	offset.Write(0);
	index.Write(0);

	dataword.Write(0x6eadbeefU);
	dataline.Write(0x123456789abcdef);
	test.Update();
	success &= TestState(i++, 0, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x0123456789abcdef, test.Line().ReadLong(), verbosity);

	ww.Set(true);
	test.Update();
	success &= TestState(i++, 0x6eadbeef, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x012345676eadbeef, test.Line().ReadLong(), verbosity);

	offset.Write(1U);
	test.Update();
	success &= TestState(i++, 0x6eadbeef, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x6eadbeef89abcdef, test.Line().ReadLong(), verbosity);

	wb.Set(true);
	test.Update();
	success &= TestState(i++, 0xef, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x012345ef89abcdef, test.Line().ReadLong(), verbosity);

	index.Write(2U);
	test.Update();
	success &= TestState(i++, 0xef0000, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x01ef456789abcdef, test.Line().ReadLong(), verbosity);

	offset.Write(0);
	test.Update();
	success &= TestState(i++, 0xef0000, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x0123456789efcdef, test.Line().ReadLong(), verbosity);

	wh.Set(true);
	wb.Set(false);
	test.Update();
	success &= TestState(i++, 0xbeef0000U, test.Word().UnsignedRead(), verbosity);
	success &= TestState(i++, 0x01234567beefcdef, test.Line().ReadLong(), verbosity);

	wh.Set(false);
	ww.Set(false);
	test.Update();
	success &= TestState(i++, 0, test.Word().Read(), verbosity);
	success &= TestState(i++, 0x0123456789abcdef, test.Line().ReadLong(), verbosity);
		
	return success;
}

bool TestCache(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Cache<256, 256, 1024>* pCache = new Cache<256, 256, 1024>();
	Cache<256, 256, 1024>& test = *pCache;

	MagicBundle<32> data;
	Wire write(true);
	Wire read(false);
	Wire writebyte(false);
	Wire writehalf(false);
	MagicBundle<32> addr;
	SystemBus bus;
	test.ConnectToBus(bus);
	test.Connect(addr, data, read, write, writebyte, writehalf);

	for (int a = 0; a < 8; a++)
	{
		addr.Write(0 + 4 * a);
		data.Write(0x10000000 + 0x111111 * a);
		test.UpdateUntilNoStall();
	}

	for (int a = 0; a < 8; ++a)
	{
		addr.Write(64 + 4*a );
		data.Write(0x70000000 + 0x222222 * a);
		test.UpdateUntilNoStall();
	}
	write.Set(false);
	read.Set(true);
		
	addr.Write(4);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0x10111111, test.Out().Read(), verbosity);
	for (int a = 1; a < 8; a++)
	{
		addr.Write(4*a);
		test.UpdateUntilNoStall();
		success &= TestState(i++, true, test.CacheHit().On(), verbosity);
		success &= TestState(i++, 0x10000000 + 0x111111 * a, test.Out().Read(), verbosity);
	}
	addr.Write(88);
	test.Update();
	for (int a = 0; a < 8; a++)
	{
		addr.Write(64 + 4*a);
		test.UpdateUntilNoStall();
		success &= TestState(i++, true, test.CacheHit().On(), verbosity);
		success &= TestState(i++, 0x70000000 + 0x222222 * a, test.Out().Read(), verbosity);
	}

	data.Write(0xaaaaaaaaU);
	addr.Write(264);
	write.Set(true);
	read.Set(false);
	test.UpdateUntilNoStall();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0xaaaaaaaaU, test.Out().UnsignedRead(), verbosity);

	addr.Write(268);
	read.Set(true);
	write.Set(false);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0, test.Out().Read(), verbosity);

	addr.Write(16);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, true, test.NeedStall().On(), verbosity);
	test.UpdateUntilNoStall();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0x10444444, test.Out().Read(), verbosity);

	addr.Write(24);
	test.Update();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0x10666666, test.Out().Read(), verbosity);

	addr.Write(90);
	write.Set(true);
	read.Set(false);
	writehalf.Set(true);
	data.Write(0xbeefU);
	test.UpdateUntilNoStall();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0xbeefccccU, test.Out().UnsignedRead(), verbosity);
	
	addr.Write(265);
	writehalf.Set(false);
	writebyte.Set(true);
	data.Write(0xbeefU);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);
	success &= TestState(i++, true, test.NeedStall().On(), verbosity);
	test.UpdateUntilNoStall();
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);
	success &= TestState(i++, 0xaaaaefaaU, test.Out().UnsignedRead(), verbosity);
	
	delete pCache;

	return success;
}

bool TestCPU(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testops.vasm", dverb);
	debugger.Start();	
	success &= TestState(i++, 1887, debugger.GetRegisterVal(21), verbosity);
	success &= TestState(i++, 2438, debugger.GetRegisterVal(2), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(3), verbosity);
	success &= TestState(i++, -551, debugger.GetRegisterVal(4), verbosity);
	success &= TestState(i++, 262, debugger.GetRegisterVal(5), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(6), verbosity);
	success &= TestState(i++, 3801, debugger.GetRegisterVal(7), verbosity);
	success &= TestState(i++, -4064, debugger.GetRegisterVal(8), verbosity);
	success &= TestState(i++, 1, debugger.GetRegisterVal(9), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(10), verbosity);
	success &= TestState(i++, -2439, debugger.GetRegisterVal(12), verbosity);
	success &= TestState(i++, 5559, debugger.GetRegisterVal(13), verbosity);
	success &= TestState(i++, -2439, debugger.GetRegisterVal(14), verbosity);
	success &= TestState(i++, 2, debugger.GetRegisterVal(15), verbosity);
	success &= TestState(i++, 4063, debugger.GetRegisterVal(16), verbosity);
	success &= TestState(i++, 2571, debugger.GetRegisterVal(17), verbosity);
	success &= TestState(i++, 0x10000004, debugger.GetRegisterVal(18), verbosity);
	success &= TestState(i++, 0, debugger.GetRegisterVal(19), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(22), verbosity);
	success &= TestState(i++, 1887, debugger.GetMemoryWord(16), verbosity);
	success &= TestState(i++, 1887, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, 4325, debugger.GetRegisterVal(20), verbosity);
	success &= TestState(i++, 1879070428, debugger.GetRegisterVal(23), verbosity);
	success &= TestState(i++, 347, debugger.GetRegisterVal(24), verbosity);
	success &= TestState(i++, 40704, debugger.GetRegisterVal(25), verbosity);
	success &= TestState(i++, 636, debugger.GetRegisterVal(26), verbosity);
	success &= TestState(i++, 19, debugger.GetRegisterVal(27), verbosity);
	success &= TestState(i++, 39, debugger.GetRegisterVal(28), verbosity);
	success &= TestState(i++, -35, debugger.GetRegisterVal(29), verbosity);
	success &= TestState(i++, -138, debugger.GetRegisterVal(30), verbosity);
	return success;
}


bool TestCPUBranch(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testbranch.vasm", dverb);
	debugger.Start();
	success &= TestState(i++, 12, debugger.GetRegisterVal(8), verbosity);
	success &= TestState(i++, 7, debugger.GetRegisterVal(9), verbosity);
	success &= TestState(i++, 300, debugger.GetRegisterVal(10), verbosity);
	success &= TestState(i++, 368, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, 332, debugger.GetRegisterVal(31), verbosity);
	success &= TestState(i++, 384, debugger.GetNextPCAddr(), verbosity);

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
	success &= TestState(i++, 2468, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(12), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(13), verbosity);
	success &= TestState(i++, 2468, debugger.GetRegisterVal(14), verbosity);
	success &= TestState(i++, 166, debugger.GetRegisterVal(20), verbosity);
	success &= TestState(i++, 123, debugger.GetRegisterVal(21), verbosity);
	success &= TestState(i++, 1234, debugger.GetMemoryWord(4), verbosity);
	success &= TestState(i++, 160, debugger.GetNextPCAddr(), verbosity);

	return success;
}

bool TestCPUMemory(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("testmemops.vasm", dverb);
	debugger.Start();
	success &= TestState(i++, 0x11aadd33, debugger.GetMemoryWord(4), verbosity);
	success &= TestState(i++, 0x11aadd33, debugger.GetRegisterVal(2), verbosity);
	success &= TestState(i++, 0x33, debugger.GetRegisterVal(3), verbosity);
	success &= TestState(i++, 0xdd, debugger.GetRegisterVal(4), verbosity);
	success &= TestState(i++, 0xaa, debugger.GetRegisterVal(5), verbosity);
	success &= TestState(i++, 0x11, debugger.GetRegisterVal(6), verbosity);
	success &= TestState(i++, 0xdd33, debugger.GetRegisterVal(7), verbosity);
	success &= TestState(i++, 0x11aa, debugger.GetRegisterVal(8), verbosity);
	success &= TestState(i++, 0x11aadd33, debugger.GetRegisterVal(9), verbosity);
	success &= TestState(i++, 0x33, debugger.GetRegisterVal(10), verbosity);
	success &= TestState(i++, -35, debugger.GetRegisterVal(11), verbosity);
	success &= TestState(i++, -86, debugger.GetRegisterVal(12), verbosity);
	success &= TestState(i++, 0x11, debugger.GetRegisterVal(13), verbosity);
	success &= TestState(i++, -8909, debugger.GetRegisterVal(14), verbosity);
	success &= TestState(i++, 0x11aa, debugger.GetRegisterVal(15), verbosity);
	success &= TestState(i++, 4, debugger.GetMemoryWord(8), verbosity);
	success &= TestState(i++, 4, debugger.GetRegisterVal(16), verbosity);
	success &= TestState(i++, 4, debugger.GetRegisterVal(17), verbosity);
	success &= TestState(i++, 4, debugger.GetRegisterVal(18), verbosity);
	success &= TestState(i++, 4, debugger.GetRegisterVal(19), verbosity);
	success &= TestState(i++, 12345, debugger.GetRegisterVal(20), verbosity);
	success &= TestState(i++, 98765, debugger.GetRegisterVal(21), verbosity);
	success &= TestState(i++, 98765, debugger.GetRegisterVal(25), verbosity);

	success &= TestState(i++, (int)0xaaaadd33, debugger.GetMemoryWord(12), verbosity);
	success &= TestState(i++, 0x33333333, debugger.GetMemoryWord(16), verbosity);
	success &= TestState(i++, 0x0033dd33, debugger.GetMemoryWord(20), verbosity);
	success &= TestState(i++, 0x11aadddd, debugger.GetMemoryWord(24), verbosity);
	success &= TestState(i++, 98765, debugger.GetMemoryWord(32), verbosity);
	return success;
}

bool TestCPUStrCpy(Verbosity verbosity, Debugger::Verbosity dverb)
{
	int i = 0;
	bool success = true;

	Debugger debugger("teststrcpy.vasm", dverb);
	debugger.Start();
	success &= TestState(i++, std::string("Hello World!"), debugger.GetMemoryString(40), verbosity);

	return success;
}

bool RunCPUTests()
{
	bool success = true;
	auto default_verb = Debugger::VERBOSE;
	RUN_TEST(TestOpcodeDecoder, FAIL_ONLY);
	RUN_TEST(TestByteMask, FAIL_ONLY);
	RUN_TEST(TestCacheLineMasker, FAIL_ONLY);
	//RUN_TEST(TestCache, FAIL_ONLY);
	RUN_TEST2(TestCPU, FAIL_ONLY, default_verb);
	RUN_TEST2(TestCPUPipelineHazards, FAIL_ONLY, default_verb);
	RUN_TEST2(TestCPUBranch, FAIL_ONLY, default_verb);
	RUN_TEST2(TestCPUMemory, FAIL_ONLY, default_verb);
	RUN_TEST2(TestCPUStrCpy, FAIL_ONLY, default_verb);

	return success;
}
