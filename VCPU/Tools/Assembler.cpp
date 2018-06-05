#include "Assembler.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


Assembler::Assembler(const std::string& filename)
	: mSourceFilename(filename)
{
	ParseSource();
}

void Assembler::ParseSource()
{
	std::ifstream file(mSourceFilename);
	std::string line;
	while (std::getline(file, line))
	{
		mSource.push_back(line);
		mBinary.push_back(ParseLine(line));
	}
}

const std::string Assembler::GetSourceLine(unsigned int line)  const
{ 
	if (line >= mSource.size())
	{
		return "";
	}
	std::string sourceline = mSource[line];
	size_t start_pos = sourceline.find("\t\t");
	if (start_pos != std::string::npos)
	{
		sourceline.replace(start_pos, 2, "\t");
	}
	return sourceline; 
}

std::vector<std::string> split(const char *str)
{
	std::vector<std::string> result;
	do
	{
		while (*str && (*str == ' ' || *str == '\t'))
			str++;

		const char *begin = str;
		while (*str != ' ' && *str != '\t' && *str)
			str++;

		result.push_back(std::string(begin, str));
	} while (0 != *str++);

	return result;
}

unsigned int Assembler::ParseLine(const std::string& line)
{
	// Formats we can take:
	// opn	$1, $2, $3		
	// opn	$1, $2, shamt	
	// opn	$1, $2, imm		
	// opn	$1, imm($2)		
	// opn	$1, 2			
	// opn	$1, $2
	// opn  $1
	// opn	$1, $2, $3
	std::string myline = line.substr(0, line.find(';'));

	std::vector<std::string> words = split(myline.c_str());
	std::string opcode = words[0];

	unsigned char rs = 0;
	unsigned char rt = 0;
	unsigned char rd = 0;
	unsigned char shamt = 0;
	int imm = 0;
	int addr = 0;
	
	auto[op, func, type] = opcodeInfo.find(opcode)->second;
	if (type == R_TYPE)
	{
		int val1, val2, val3;

		if (sscanf_s(words[1].c_str(), "$%d", &val1))
		{
			// Got register 1
			if (words.size() > 2 && sscanf_s(words[2].c_str(), "$%d", &val2))
			{
				// Got register 2
				if (sscanf_s(words[3].c_str(), "$%d", &val3))
				{
					// $rd, $rs, $rt
					rd = val1;
					rs = val2;
					rt = val3;
				}
				else if (sscanf_s(words[3].c_str(), "%d", &val3))
				{
					// $rd, $rs, shamt
					rd = val1;
					rs = val2;
					shamt = val3;
				}
				else
				{
					// $rs, $rt
					rs = val1;
					rt = val2;
				}
			}
			else
			{
				// val1
				// $rs
				rs = val1;
			}
		}
		else
		{
			std::cout << "ERROR: Couldn't parse line " << line << std::endl;
		}
		return (op << 26) + (rs << 21) + (rt << 16) + (rd << 11) + (shamt << 6) + func;
	}
	else if (type == I_TYPE)
	{
		int val1, val2, val3;

		// Try $R1
		if (sscanf_s(words[1].c_str(), "$%d", &val1))
		{
			// Try $R2
			if (sscanf_s(words[2].c_str(), "$%d", &val2))
			{
				// Try $R3
				if (sscanf_s(words[3].c_str(), "%d", &val3))
				{
					// $rt, $rs, imm
					rt = val1;
					rs = val2;
					imm = val3;
				}
				else
				{
					std::cout << "ERROR: Couldn't parse line " << line << std::endl;
				}
			}
			else
			{
				int got_vals = sscanf_s(words[2].c_str(), "%d($%d)", &val2, &val3);
				if (got_vals == 2)
				{
					// $rt, imm($rs)
					rt = val1;
					imm = val2;
					rs = val3;
				}
				else if (got_vals == 1)
				{
					// $rs, imm
					rs = val1;
					imm = val2;
				}
				else
				{
					std::cout << "ERROR: Couldn't parse line " << line << std::endl;
				}
			}
		}
		else
		{
			std::cout << "ERROR: Couldn't parse line " << line << std::endl;
		}
		return (op << 26) + (rs << 21) + (rt << 16) + (imm & 0x0000FFFF);
	}
	else if (type == J_TYPE)
	{
		if (!sscanf_s(words[1].c_str(), "%d", &addr))
		{
			std::cout << "ERROR: Couldn't parse line " << line << std::endl;
		}		
		return (op << 26) + addr;
	}
	else if (type == X_TYPE)
	{
		return (op << 26);
	}
	else
	{
		std::cout << "ERROR: No type information for line " << line << std::endl;
	}

	return 0;
}

const std::map<std::string, std::tuple<unsigned char, unsigned char, InstType>> Assembler::opcodeInfo(
	{
// Opcode Name, Opc, Func
	{ "sll"   , { 0,  0,  R_TYPE } } ,		// TESTED
	{ "srl"   , { 0,  2,  R_TYPE } } ,		// TESTED
	{ "sra"   , { 0,  3,  R_TYPE } } ,		// TESTED
	{ "sllv"  , { 0,  4,  R_TYPE } } ,		// TESTED
	{ "srlv"  , { 0,  6,  R_TYPE } } ,		// TESTED
	{ "srav"  , { 0,  7,  R_TYPE } } ,		// TESTED
	{ "jr"    , { 0,  8,  R_TYPE } } ,		// TESTED
	{ "jalr"  , { 0,  9,  R_TYPE } } ,		// TESTED
	{ "mfhi"  , { 0,  16, R_TYPE } } ,
	{ "mthi"  , { 0,  17, R_TYPE } } ,
	{ "mflo"  , { 0,  18, R_TYPE } } ,
	{ "mtlo"  , { 0,  19, R_TYPE } } ,
	{ "mult"  , { 0,  24, R_TYPE } } ,
	{ "multu" , { 0,  25, R_TYPE } } ,
	{ "div"   , { 0,  26, R_TYPE } } ,
	{ "divu"  , { 0,  27, R_TYPE } } ,
	{ "add"   , { 0,  32, R_TYPE } } ,		// TESTED
	{ "addu"  , { 0,  33, R_TYPE } } ,
	{ "sub"   , { 0,  34, R_TYPE } } ,		// TESTED
	{ "subu"  , { 0,  35, R_TYPE } } ,
	{ "and"   , { 0,  36, R_TYPE } } ,		// TESTED
	{ "or"    , { 0,  37, R_TYPE } } ,		// TESTED
	{ "xor"   , { 0,  38, R_TYPE } } ,		// TESTED
	{ "nor"   , { 0,  39, R_TYPE } } ,		// TESTED
	{ "slt"   , { 0,  42, R_TYPE } } ,		// TESTED
	{ "sltu"  , { 0,  43, R_TYPE } } ,
	{ "j"	  , { 2,  0,  J_TYPE } } ,		// TESTED
	{ "jal"	  , { 3,  0,  J_TYPE } } ,		// TESTED
	{ "beq"   , { 4,  0,  I_TYPE } } ,		// TESTED
	{ "bne"   , { 5,  0,  I_TYPE } } ,		// TESTED
	{ "blez"  , { 6,  0,  I_TYPE } } ,		// TESTED
	{ "bgtz"  , { 7,  0,  I_TYPE } } ,		// TESTED
	{ "addi"  , { 8,  0,  I_TYPE } } ,		// TESTED
	{ "addiu" , { 9,  0,  I_TYPE } } ,
	{ "slti"  , { 10, 0,  I_TYPE } } ,		// TESTED
	{ "sltiu" , { 11, 0,  I_TYPE } } ,
	{ "andi"  , { 12, 0,  I_TYPE } } ,		// TESTED
	{ "ori"   , { 13, 0,  I_TYPE } } ,		// TESTED
	{ "xori"  , { 14, 0,  I_TYPE } } ,		// TESTED
	{ "lui"   , { 15, 0,  I_TYPE } } ,
	{ "lb"    , { 32, 0,  I_TYPE } } ,
	{ "lh"    , { 33, 0,  I_TYPE } } ,
	{ "lwl"   , { 34, 0,  I_TYPE } } ,
	{ "lw"    , { 35, 0,  I_TYPE } } ,		// TESTED
	{ "lbu"   , { 36, 0,  I_TYPE } } ,
	{ "lhu"   , { 37, 0,  I_TYPE } } ,
	{ "lwr"   , { 38, 0,  I_TYPE } } ,
	{ "sb"    , { 40, 0,  I_TYPE } } ,
	{ "sh"    , { 41, 0,  I_TYPE } } ,
	{ "swl"   , { 42, 0,  I_TYPE } } ,
	{ "sw"    , { 43, 0,  I_TYPE } } ,		// TESTED
	{ "swr"   , { 46, 0,  I_TYPE } } ,
	{ "hlt"   , { 63, 0,  X_TYPE } }		// TESTED
	});