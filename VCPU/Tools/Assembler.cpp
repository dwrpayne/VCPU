#include "Assembler.h"
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>


Assembler::Assembler(const std::string& filename)
	: mSourceFilename(filename)
{
	ParseSource();
}

void Assembler::ParseSource()
{
	std::ifstream file(mSourceFilename);
	std::string line;
	unsigned int source_line_num = 0;
	while (std::getline(file, line))
	{
		mSource.push_back(line);
		for (auto& line : ParseLine(line))
		{
			mAssembled.push_back(line);
			mBinary.push_back(GetMachineLanguage(line));
			mSourceLine.push_back(source_line_num);
		}
		source_line_num++;
	}
}

const std::string Assembler::GetSourceLine(unsigned int line)  const
{ 
	if (line >= mSource.size())
	{
		return "";
	}
	std::string sourceline = mAssembled[line];
	size_t start_pos = sourceline.find("\t\t");
	if (start_pos != std::string::npos)
	{
		sourceline.replace(start_pos, 2, "\t");
	}
	sourceline.append("\t| ");
	sourceline.append(std::to_string(mSourceLine[line]));
	sourceline.append(" ");
	sourceline.append(mSource[mSourceLine[line]]);
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

std::vector<std::string> Assembler::ParseLine(const std::string& l)
{
	std::string line = l.substr(0, l.find(';'));

	// TODO: hex number replace
	
	// TODO: label searching (after pseudo-expansion)
	
	// Pseudo-Instructions
	// Logic and Data
	line = std::regex_replace(line, std::regex("nop"),								"sll	$$zero, $$zero, 0");
	line = std::regex_replace(line, std::regex("not\\s+(\\$.+), (\\$.+)"),			"nor	$1, $2, $$zero");
	line = std::regex_replace(line, std::regex("move\\s+(\\$.+), (\\$.+)"),			"addiu	$1, $2, 0");
	line = std::regex_replace(line, std::regex("clr\\s+(\\$.+)"),					"addu	$1, $$zero, $$zero");
	line = std::regex_replace(line, std::regex("push\\s+(\\$.+)"),					"addi	$$sp, $$sp, -4\nsw	$1, 0($$sp)");
	line = std::regex_replace(line, std::regex("pop\\s+(\\$.+)"),					"lw		$1, 0($$sp)\naddi $$sp, $$sp, 4");

	// Load Immediate, needs to check size of immediate op.
	std::regex li_regex("li\\s+(\\$.+), (\\d+)");
	std::smatch li_match;
	if (std::regex_search(line, li_match, li_regex))
	{
		std::string reg = li_match[1];
		unsigned int val = std::stoul(li_match[2]);
		std::stringstream ss;
		if (val > 65535)
		{
			ss << "lui	" << reg << ", " << (val >> 16) << std::endl;
		}
		ss << "ori	" << reg << ", " << reg << ", " << (val & 0xffff);
		line = ss.str();
	}
	
	
	// Branch
	line = std::regex_replace(line, std::regex("b\\s+(\\d+)"),						"b		$$zero, $$zero, $1");
	line = std::regex_replace(line, std::regex("bgt\\s+(\\$.+), (\\$.+), (\\d+)"),	"slt	$$at, $2, $1\nbne	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("blt\\s+(\\$.+), (\\$.+), (\\d+)"),	"slt	$$at, $1, $2\nbne	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("bge\\s+(\\$.+), (\\$.+), (\\d+)"),	"slt	$$at, $1, $2\nbeq	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("ble\\s+(\\$.+), (\\$.+), (\\d+)"),	"slt	$$at, $2, $1\nbeq	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("beqz\\s+(\\$.+), (\\d+)"),			"beq	$1, $$zero, $2");
	line = std::regex_replace(line, std::regex("beq\\s+(\\$.+), (\\d+), (\\d+)"),	"ori	$$at, $$zero, $2\nbeq	$1, $$at, $3");
	line = std::regex_replace(line, std::regex("bne\\s+(\\$.+), (\\d+), (\\d+)"),	"ori	$$at, $$zero, $2\nbeq	$1, $$at, $3");




	// Register Mnemonics
	static const std::array<std::string, 32> reg_names = { 
		"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
		"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
		"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
		"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra" 
	};

	for (int i = 0; i < 32; i++)
	{
		std::string rep = "$$" + std::to_string(i);
		line = std::regex_replace(line, std::regex("\\$"+reg_names[i]), rep);
	}

	std::vector<std::string> lines;
	std::stringstream ss(line);
	std::string buf;
	while (std::getline(ss, buf))
	{ 
		lines.push_back(buf);
	}
	return lines;
}

unsigned int Assembler::GetMachineLanguage(const std::string& line)
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

	std::vector<std::string> words = split(line.c_str());
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