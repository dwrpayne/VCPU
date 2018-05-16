#include "Instruction.h"

Instruction::Instruction(const std::string& name, int rs, int rt, int rd, int shamt)
	: mName(name)
	, mRS(rs)
	, mRT(rt)
	, mRD(rd)
	, mShAmt(shamt)
	, mImm(0)
	, mAddr(0)
	, mType(TYPE_R)
{
	auto [op, func] = opcodeInfo.find(name)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(const std::string& name, int rs, int rt, int imm)
	: mName(name)
	, mRS(rs)
	, mRT(rt)
	, mRD(0)
	, mShAmt(0) 
	, mFunc(0)
	, mImm(imm)
	, mAddr(0)
	, mType(TYPE_I)
{
	auto[op, func] = opcodeInfo.find(name)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + (imm & 0x0000FFFF);
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(const std::string& name, int addr)
	: mName(name)
	, mRS(0)
	, mRT(0)
	, mRD(0)
	, mShAmt(0)
	, mFunc(0)
	, mImm(0)
	, mAddr(addr)
	, mType(TYPE_J)
{
	auto[op, func] = opcodeInfo.find(name)->second;
	mVal = (op << 26) + addr;
	mFunc = func;
	mOp = op;
}

std::ostream& operator<<(std::ostream& os, const Instruction& i)
{
	if (i.mType == Instruction::TYPE_J)
	{
		os << i.mName << "\t" << i.mAddr;
	}
	else if (i.mType == Instruction::TYPE_I)
	{
		os << i.mName << "\t$" << (int)i.mRS << " $" << (int)i.mRT << " " << i.mImm;
	}
	else if (i.mType == Instruction::TYPE_R)
	{
		os << i.mName << "\t$" << (int)i.mRS << " $" << (int)i.mRT << " $" << (int)i.mRD << " " << (int)i.mShAmt << " ";
	}
	return os;
}

const std::map<std::string, std::tuple<unsigned char, unsigned char>> Instruction::opcodeInfo(
{
	{ "sll"   , {   0,    0,   } } ,
	{ "srl"   , {   0,    2,   } } ,
	{ "sra"   , {   0,    3,   } } ,
	{ "sllv"  , {   0,    4,    } } ,
	{ "srlv"  , {   0,    6,    } } ,
	{ "srav"  , {   0,    7,    } } ,
	{ "jr"    , {   0,    8,  } } ,
	{ "jalr"  , {   0,    9,    } } ,
	{ "mfhi"  , {   0,    16     } } ,
	{ "mthi"  , {   0,    17     } } ,
	{ "mflo"  , {   0,    18     } } ,
	{ "mtlo"  , {   0,    19     } } ,
	{ "mult"  , {   0,    24     } } ,
	{ "multu" , {   0,    25      } } ,
	{ "div"   , {   0,    26    } } ,
	{ "divu"  , {   0,    27     } } ,
	{ "add"   , {   0,    32    } } ,
	{ "addu"  , {   0,    33     } } ,
	{ "sub"   , {   0,    34    } } ,
	{ "subu"  , {   0,    35     } } ,
	{ "and"   , {   0,    36    } } ,
	{ "or"    , {   0,    37   } } ,
	{ "xor"   , {   0,    38    } } ,
	{ "nor"   , {   0,    39    } } ,
	{ "slt"   , {   0,    42    } } ,
	{ "sltu"  , {   0,    43     } } ,
	{ "bltz"  , {   1,    0,    } } ,
	{ "bgez"  , {   1,    0,    } } ,
	{ "bltzal", {   1,    0,      } } ,
	{ "bgezal", {   1,    0,	} } ,
	{ "j"	 ,	 {   2,    0,	} } ,
	{ "jal"	,	{   3,    0,	} } ,
	{ "beq"  ,  {   4,    0,   } } ,
	{ "bne"  ,  {   5,    0,   } } ,
	{ "blez" ,  {   6,    0,    } } ,
	{ "bgtz" ,  {   7,    0,    } } ,
	{ "addi" ,  {   8,    0,    } } ,
	{ "addiu",  {   9,    0,     } } ,
	{ "slti" ,  {   10,   0,     } } ,
	{ "sltiu",  {   11,   0,      } } ,
	{ "andi" ,  {   12,   0,     } } ,
	{ "ori"  ,  {   13,   0,    } } ,
	{ "xori" ,  {   14,   0,     } } ,
	{ "lui"  ,  {   15,   0,    } } ,
	{ "lb"   ,  {   32,   0,   } } ,
	{ "lh"   ,  {   33,   0,   } } ,
	{ "lwl"  ,  {   34,   0,    } } ,
	{ "lw"   ,  {   35,   0,   } } ,
	{ "lbu"  ,  {   36,   0,    } } ,
	{ "lhu"  ,  {   37,   0,    } } ,
	{ "lwr"  ,  {   38,   0,    } } ,
	{ "sb"   ,  {   40,   0,   } } ,
	{ "sh"   ,  {   41,   0,   } } ,
	{ "swl"  ,  {   42,   0,    } } ,
	{ "sw"   ,  {   43,   0,   } } ,
	{ "swr"  ,  {   46,   0,    } }
	});