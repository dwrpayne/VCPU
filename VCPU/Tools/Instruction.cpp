#include "Instruction.h"

Instruction::Instruction(EOpcode opcode, int rs, int rt, int rd, int shamt)
	: mOpcode(opcode)
	, mRS(rs)
	, mRT(rt)
	, mRD(rd)
	, mShAmt(shamt)
	, mImm(0)
	, mAddr(0)
	, mType(TYPE_R)
{
	auto [op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
	mName = name;
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(EOpcode opcode, int rs, int rt, int imm)
	: mOpcode(opcode)
	, mRS(rs)
	, mRT(rt)
	, mRD(0)
	, mShAmt(0) 
	, mFunc(0)
	, mImm(imm)
	, mAddr(0)
	, mType(TYPE_I)
{
	auto[op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + (imm & 0x0000FFFF);
	mName = name;
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(EOpcode opcode, int addr)
	: mOpcode(opcode)
	, mRS(0)
	, mRT(0)
	, mRD(0)
	, mShAmt(0)
	, mFunc(0)
	, mImm(0)
	, mAddr(addr)
	, mType(TYPE_J)
{
	auto[op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + addr;
	mName = name;
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
		os << i.mName << "\t$" << (int)i.mRS << "\t$" << (int)i.mRT << "\t" << i.mImm;
	}
	else if (i.mType == Instruction::TYPE_R)
	{
		os << i.mName << "\t$" << (int)i.mRS << "\t$" << (int)i.mRT << "\t$" << (int)i.mRD << "\t" << (int)i.mShAmt << "\t";
	}
	return os;
}

const std::map<EOpcode, std::tuple<unsigned char, unsigned char, const char*>> Instruction::opcodeInfo(
{
	{ SLL,	  {   0,    0,  "sll"      }	 } ,
	{ SRL,	  {   0,    2,  "srl"      }	 } ,
	{ SRA,	  {   0,    3,  "sra"      }	 } ,
	{ SLLV,	  {   0,    4,  "sllv"      }	 } ,
	{ SRLV,	  {   0,    6,  "srlv"      }	 } ,
	{ SRAV,	  {   0,    7,  "srav"      }	 } ,
	{ JR,	  {   0,    8,  "jr"      }	 } ,
	{ JALR,	  {   0,    9,  "jalr"      }	 } ,
	{ MFHI,	  {   0,    16, "mfhi"       }	 } ,
	{ MTHI,	  {   0,    17, "mthi"       }	 } ,
	{ MFLO,	  {   0,    18, "mflo"       }	 } ,
	{ MTLO,	  {   0,    19, "mtlo"       }	 } ,
	{ MULT,	  {   0,    24, "mult"       }	 } ,
	{ MULTU,  {   0,    25, "multu"       } } ,
	{ DIV,	  {   0,    26, "div"       }	 } ,
	{ DIVU,	  {   0,    27, "divu"       }	 } ,
	{ ADD,	  {   0,    32, "add"       }	 } ,
	{ ADDU,	  {   0,    33, "addu"       }	 } ,
	{ SUB,	  {   0,    34, "sub"       }	 } ,
	{ SUBU,	  {   0,    35, "subu"       }	 } ,
	{ AND,	  {   0,    36, "and"       }	 } ,
	{ OR,	  {   0,    37, "or"       }	 } ,
	{ XOR,	  {   0,    38, "xor"       }	 } ,
	{ NOR,	  {   0,    39, "nor"       }	 } ,
	{ SLT,	  {   0,    42, "slt"       }	 } ,
	{ SLTU,	  {   0,    43, "sltu"       }	 } ,
	{ BLTZ,	  {   1,    0,  "bltz"      }	 } ,
	{ BGEZ,	  {   1,    0,  "bgez"      }	 } ,
	{ BLTZAL, {   1,    0,  "bltzal"      } } ,
	{ BGEZAL, {   1,    0,  "bgezal"	} } ,
	{ J,	  {   2,    0,  "j"			} } ,
	{ JAL,	  {   3,    0,  "jal"		} } ,
	{ BEQ,	  {   4,    0,  "beq"      }	 } ,
	{ BNE,	  {   5,    0,  "bne"      }	 } ,
	{ BLEZ,	  {   6,    0,  "blez"      }	 } ,
	{ BGTZ,	  {   7,    0,  "bgtz"      }	 } ,
	{ ADDI,	  {   8,    0,  "addi"      }	 } ,
	{ ADDIU,  {   9,    0,  "addiu"      }	 } ,
	{ SLTI,	  {   10,   0,  "slti"       }	 } ,
	{ SLTIU,  {   11,   0,  "sltiu"       } } ,
	{ ANDI,	  {   12,   0,  "andi"       }	 } ,
	{ ORI,	  {   13,   0,  "ori"       }	 } ,
	{ XORI,	  {   14,   0,  "xori"       }	 } ,
	{ LUI,	  {   15,   0,  "lui"       }	 } ,
	{ LB,	  {   32,   0,  "lb"       }	 } ,
	{ LH,	  {   33,   0,  "lh"       }	 } ,
	{ LWL,	  {   34,   0,  "lwl"       }	 } ,
	{ LW,	  {   35,   0,  "lw"       }	 } ,
	{ LBU,	  {   36,   0,  "lbu"       }	 } ,
	{ LHU,	  {   37,   0,  "lhu"       }	 } ,
	{ LWR,	  {   38,   0,  "lwr"       }	 } ,
	{ SB,	  {   40,   0,  "sb"       }	 } ,
	{ SH,	  {   41,   0,  "sh"       }	 } ,
	{ SWL,	  {   42,   0,  "swl"       }	 } ,
	{ SW,	  {   43,   0,  "sw"       }	 } ,
	{ SWR,	  {   46,   0,  "swr"       }	 }
});