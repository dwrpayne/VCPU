#pragma once
#include <map>
#include <string>
#include <iostream>
#include "CPU/Instructions.h"

class Instruction
{
public:
	enum Type
	{
		TYPE_R,
		TYPE_I,
		TYPE_J
	};

	Instruction(const std::string& name, int rs, int rt, int rd, int shamt);
	Instruction(const std::string& name, int rs, int rt, int imm);
	Instruction(const std::string& name, int addr);

	unsigned int GetValue() const { return mVal; }
	friend std::ostream& operator<<(std::ostream& os, const Instruction& i);

private:
	unsigned int mVal;
	unsigned char mOp;
	unsigned char mRS, mRT, mRD;
	unsigned char mShAmt, mFunc;
	int mImm;
	int mAddr;
	const std::string mName;
	Type mType;

	static const std::map <std::string, std::tuple<unsigned char, unsigned char>> opcodeInfo;
};

std::ostream& operator<<(std::ostream& os, const Instruction& i);