#pragma once

enum Opcode : unsigned char
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


enum Function : unsigned char
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
};
