#pragma once
#include <map>
#include <string>
#include "CPU/Instructions.h"


enum EOpcode : unsigned char
{
	SLL,
	SRL,
	SRA,
	SLLV,
	SRLV,
	SRAV,

	JR,
	JALR,

	MFHI,
	MTHI,
	MFLO,
	MTLO,
	MULT,
	MULTU,
	DIV,
	DIVU,

	ADD,
	ADDU,
	SUB,
	SUBU,
	AND,
	OR,
	XOR,
	NOR,
	SLT,
	SLTU,

	BLTZ,
	BGEZ,
	BLTZAL,
	BGEZAL,
	J,
	JAL,
	BEQ,
	BNE,
	BLEZ,
	BGTZ,

	ADDI,
	ADDIU,
	SLTI,
	SLTIU,
	ANDI,
	ORI,
	XORI,
	LUI,

	LB,
	LH,
	LWL,
	LW,
	LBU,
	LHU,
	LWR,
	SB,
	SH,
	SWL,
	SW,
	SWR,
};


class Instruction
{
public:
	Instruction(EOpcode opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt);
	Instruction(EOpcode opcode, unsigned int rs, unsigned int rt, unsigned int imm);
	Instruction(EOpcode opcode, unsigned int addr);

	unsigned int GetValue() { return mVal; }

private:
	unsigned int mVal;
	EOpcode mOpcode;
	unsigned char mOp;
	unsigned char mRS, mRT, mRD;
	unsigned char mShAmt, mFunc;
	unsigned short mImm;
	unsigned int mAddr;
	const char* mName;

	static const std::map < EOpcode, std::tuple<unsigned char, unsigned char, const char*>> opcodeInfo;
};
