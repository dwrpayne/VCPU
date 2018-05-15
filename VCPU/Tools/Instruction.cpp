#include "Instruction.h"

Instruction::Instruction(EOpcode opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt)
	: mOpcode(opcode)
	, mRS(rs)
	, mRT(rt)
	, mRD(rd)
	, mShAmt(shamt)
	, mImm(0)
	, mAddr(0)
{
	auto [op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
	mName = name;
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(EOpcode opcode, unsigned int rs, unsigned int rt, unsigned int imm)
	: mOpcode(opcode)
	, mRS(rs)
	, mRT(rt)
	, mRD(0)
	, mShAmt(0) 
	, mFunc(0)
	, mImm(imm)
	, mAddr(0)
{
	auto[op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + (rs << 21) + (rt << 16) + imm;
	mName = name;
	mFunc = func;
	mOp = op;
}

Instruction::Instruction(EOpcode opcode, unsigned int addr)
	: mOpcode(opcode)
	, mRS(0)
	, mRT(0)
	, mRD(0)
	, mShAmt(0)
	, mFunc(0)
	, mImm(0)
	, mAddr(addr)
{
	auto[op, func, name] = opcodeInfo.find(opcode)->second;
	mVal = (op << 26) + addr;
	mName = name;
	mFunc = func;
	mOp = op;
}

const std::map<EOpcode, std::tuple<unsigned char, unsigned char, const char*>> Instruction::opcodeInfo(
{
	{ SLL,	  {   0,    0,       "SLL"      }	 } ,
	{ SRL,	  {   0,    2,       "SRL"      }	 } ,
	{ SRA,	  {   0,    3,       "SRA"      }	 } ,
	{ SLLV,	  {   0,    4,       "SLLV"      }	 } ,
	{ SRLV,	  {   0,    6,       "SRLV"      }	 } ,
	{ SRAV,	  {   0,    7,       "SRAV"      }	 } ,
	{ JR,	  {   0,    8,       "JR"      }	 } ,
	{ JALR,	  {   0,    9,       "JALR"      }	 } ,
	{ MFHI,	  {   0,    16,      "MFHI"       }	 } ,
	{ MTHI,	  {   0,    17,      "MTHI"       }	 } ,
	{ MFLO,	  {   0,    18,      "MFLO"       }	 } ,
	{ MTLO,	  {   0,    19,      "MTLO"       }	 } ,
	{ MULT,	  {   0,    24,      "MULT"       }	 } ,
	{ MULTU,  {   0,    25,      "MULTU"       } } ,
	{ DIV,	  {   0,    26,      "DIV"       }	 } ,
	{ DIVU,	  {   0,    27,      "DIVU"       }	 } ,
	{ ADD,	  {   0,    32,      "ADD"       }	 } ,
	{ ADDU,	  {   0,    33,      "ADDU"       }	 } ,
	{ SUB,	  {   0,    34,      "SUB"       }	 } ,
	{ SUBU,	  {   0,    35,      "SUBU"       }	 } ,
	{ AND,	  {   0,    36,      "AND"       }	 } ,
	{ OR,	  {   0,    37,      "OR"       }	 } ,
	{ XOR,	  {   0,    38,      "XOR"       }	 } ,
	{ NOR,	  {   0,    39,      "NOR"       }	 } ,
	{ SLT,	  {   0,    42,      "SLT"       }	 } ,
	{ SLTU,	  {   0,    43,      "SLTU"       }	 } ,
	{ BLTZ,	  {   1,    0,       "BLTZ"      }	 } ,
	{ BGEZ,	  {   1,    0,       "BGEZ"      }	 } ,
	{ BLTZAL, {   1,    0,       "BLTZAL"      } } ,
	{ BGEZAL, {   1,    0,       "BGEZAL"      } } ,
	{ J,	  {   2,    0,       "J"      }		 } ,
	{ JAL,	  {   3,    0,       "JAL"      }	 } ,
	{ BEQ,	  {   4,    0,       "BEQ"      }	 } ,
	{ BNE,	  {   5,    0,       "BNE"      }	 } ,
	{ BLEZ,	  {   6,    0,       "BLEZ"      }	 } ,
	{ BGTZ,	  {   7,    0,       "BGTZ"      }	 } ,
	{ ADDI,	  {   8,    0,       "ADDI"      }	 } ,
	{ ADDIU,  {   9,    0,       "ADDIU"      }	 } ,
	{ SLTI,	  {   10,   0,       "SLTI"       }	 } ,
	{ SLTIU,  {   11,   0,       "SLTIU"       } } ,
	{ ANDI,	  {   12,   0,       "ANDI"       }	 } ,
	{ ORI,	  {   13,   0,       "ORI"       }	 } ,
	{ XORI,	  {   14,   0,       "XORI"       }	 } ,
	{ LUI,	  {   15,   0,       "LUI"       }	 } ,
	{ LB,	  {   32,   0,       "LB"       }	 } ,
	{ LH,	  {   33,   0,       "LH"       }	 } ,
	{ LWL,	  {   34,   0,       "LWL"       }	 } ,
	{ LW,	  {   35,   0,       "LW"       }	 } ,
	{ LBU,	  {   36,   0,       "LBU"       }	 } ,
	{ LHU,	  {   37,   0,       "LHU"       }	 } ,
	{ LWR,	  {   38,   0,       "LWR"       }	 } ,
	{ SB,	  {   40,   0,       "SB"       }	 } ,
	{ SH,	  {   41,   0,       "SH"       }	 } ,
	{ SWL,	  {   42,   0,       "SWL"       }	 } ,
	{ SW,	  {   43,   0,       "SW"       }	 } ,
	{ SWR,	  {   46,   0,       "SWR"       }	 }
});