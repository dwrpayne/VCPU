#pragma once
#include <map>
#include <string>
#include <iostream>
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
	enum Type
	{
		TYPE_R,
		TYPE_I,
		TYPE_J
	};

	Instruction(EOpcode opcode, int rs, int rt, int rd, int shamt);
	Instruction(EOpcode opcode, int rs, int rt, int imm);
	Instruction(EOpcode opcode, int addr);

	unsigned int GetValue() const { return mVal; }
	friend std::ostream& operator<<(std::ostream& os, const Instruction& i);

private:
	unsigned int mVal;
	EOpcode mOpcode;
	unsigned char mOp;
	unsigned char mRS, mRT, mRD;
	unsigned char mShAmt, mFunc;
	int mImm;
	int mAddr;
	const char* mName;
	Type mType;

	static const std::map < EOpcode, std::tuple<unsigned char, unsigned char, const char*>> opcodeInfo;
};

std::ostream& operator<<(std::ostream& os, const Instruction& i);