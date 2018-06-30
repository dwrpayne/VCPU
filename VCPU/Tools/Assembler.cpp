#include "Assembler.h"
#include <string>
#include <array>
#include <vector>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <assert.h>
#include "Program.h"
#include "CPU/Addresses.h"

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

Assembler::Assembler()
	: mCurSection(Section::CODE)
{
	pProgram = new Program();
	ParseSourceLine("nop ;null address", pProgram);		// address 0x00000000 is empty.
	ParseSourceLine("li $sp, " + std::to_string(USER_STACK_START), pProgram);
	ParseSourceLine("li $gp, " + std::to_string(USER_DATA_START), pProgram);
	ParseSourceLine("call main", pProgram);
	IncludeLib("library.vasm");
}

void Assembler::IncludeLib(const std::string & filename)
{
	mIncludeFiles.push_back(filename);
}

const Program* Assembler::Assemble(const std::string& filename)
{
	ParseFile(filename, pProgram);
	for (auto inc : mIncludeFiles)
	{
		ParseFile(inc, pProgram);
	}

	pProgram->ConvertLabels();
	for (auto& ins : pProgram->Instructions())
	{
		ins.mBinary = GetMachineLanguage(ins.mText);
	}

	return pProgram;
}

void Assembler::ParseFile(const std::string & filename, Program * program)
{
	std::ifstream file("assembly\\" + filename);
	std::string line;
	while (std::getline(file, line))
	{
		ParseSourceLine(line, program);
	}
}

void Assembler::ParseSourceLine(const std::string &line, Program * program)
{
	if (line.find(".text") != std::string::npos)
	{
		mCurSection = Section::TEXT;
		return;
	}

	// Get code line without comment
	std::string code_line = line.substr(0, line.find(';'));

	// Get comment separately
	auto comment_pos = line.find(';');
	std::string comment = comment_pos == std::string::npos ? "" : line.substr(comment_pos);

	// Get label
	auto colon_pos = code_line.find(':');
	std::string label = "";
	if (colon_pos != std::string::npos)
	{
		label = code_line.substr(0, colon_pos);
		code_line = code_line.substr(colon_pos + 1);
	}

	std::vector<std::string> words = split(code_line.c_str());

	if (mCurSection == Section::CODE)
	{
		unsigned int source_line = program->AddSourceLine(label, code_line, comment);
		if (words[0].size() > 0)
		{
			for (auto& line : GetInstructionsForLine(code_line))
			{
				program->AddInstruction(source_line, line);
			}
		}
	}
	else if (mCurSection == Section::TEXT)
	{
		if (words[0].size() > 2 && words[0][0] == '.')
		{
			std::vector<unsigned char> bytes;
			if (code_line.find("ascii") != std::string::npos)
			{
				bool in_string = false;
				for (unsigned char c : code_line)
				{
					if (c == '"') 
					{
						if (in_string) break;
						in_string = true;
						continue;
					}
					if (in_string)
					{
						bytes.push_back(c);
					}
				}
				program->AddTextField(label, bytes.size(), bytes);
			}
		}
	}
	else
	{
		assert(false);
	}
}

const std::string Assembler::GetRegName(unsigned int reg) const
{
	return reg_names[reg];
}

std::vector<std::string> Assembler::GetInstructionsForLine(const std::string& l)
{
	std::string line = l;

	// TODO: hex number replaces
	for (auto& word : split(l.c_str()))
	{
		if (word.size() > 2 && word[0] == '0' && (word[1] == 'x' || word[1] == 'X'))
		{
			unsigned int hex = std::stoul(word, 0, 16);
			line = std::regex_replace(line, std::regex(word), std::to_string(hex));
			break;
		}
	}
	// Function calls
	line = std::regex_replace(line, std::regex("^\\s*call\\s+(\\S+)\\b"), "jal	$1\nnop");
	line = std::regex_replace(line, std::regex("^\\s*ret\\b"), "jr		$ra\nnop");
	line = std::regex_replace(line, std::regex("^\\s*push\\s+(\\$.+)\\b"), "addi	$$sp, $$sp, -4\nsw	$1, 0($$sp)");
	line = std::regex_replace(line, std::regex("^\\s*pop\\s+(\\$.+)\\b"), "lw		$1, 0($$sp)\naddi $$sp, $$sp, 4");
	
	// Load Immediate, needs to check size of immediate op.
	std::regex li_regex("^\\s*li\\s+(\\$.+), (\\d+)");
	std::smatch li_match;
	if (std::regex_search(line, li_match, li_regex))
	{
		std::string reg = li_match[1];
		unsigned int val = std::stoul(li_match[2]);
		std::stringstream ss;
		if (val > 65535)
		{
			ss << "lui	$at, " << (val >> 16) << std::endl;
			ss << "ori	" << reg << ", $at, " << (val & 0xffff);
		}
		else
		{
			ss << "ori	" << reg << ", $zero, " << val;
		}
		line = ss.str();
	}	
	
	// Branch
	line = std::regex_replace(line, std::regex("^\\s*b\\s+(\\S+)"),						"beq	$$zero, $$zero, $1");
	line = std::regex_replace(line, std::regex("^\\s*bgt\\s+(\\$.+), (\\$.+), (\\S+)"),	"slt	$$at, $2, $1\nbne	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("^\\s*blt\\s+(\\$.+), (\\$.+), (\\S+)"),	"slt	$$at, $1, $2\nbne	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("^\\s*bge\\s+(\\$.+), (\\$.+), (\\S+)"),	"slt	$$at, $1, $2\nbeq	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("^\\s*ble\\s+(\\$.+), (\\$.+), (\\S+)"),	"slt	$$at, $2, $1\nbeq	$$at, $$zero, $3");
	line = std::regex_replace(line, std::regex("^\\s*beqz\\s+(\\$.+), (\\S+)"),			"beq	$1, $$zero, $2");
	
	// Pseudo-Instructions
	// Logic and Data
	line = std::regex_replace(line, std::regex("^\\s*nop\\b"), "sll	$$zero, $$zero, 0");
	line = std::regex_replace(line, std::regex("^\\s*not\\s+(\\$.+), (\\$.+)\\b"), "nor	$1, $2, $$zero");
	line = std::regex_replace(line, std::regex("^\\s*move\\s+(\\$.+), (\\$.+)\\b"), "addiu	$1, $2, 0");
	line = std::regex_replace(line, std::regex("^\\s*clr\\s+(\\$.+)\\b"), "addu	$1, $$zero, $$zero");

	// Register Names
	// Special case 0 because it has two names, $r0 and $zero
	line = std::regex_replace(line, std::regex("\\$zero"), "$$0");
	for (int i = 0; i < 32; i++)
	{
		std::string rep = "$$" + std::to_string(i);
		line = std::regex_replace(line, std::regex("\\$"+reg_names[i]), rep);
	}

	// Split up the ops if we have more than one.
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
				if (words.size() > 3 && sscanf_s(words[3].c_str(), "$%d", &val3))
				{
					// $rd, $rs, $rt
					rd = val1;
					rs = val2;
					rt = val3;
				}
				else if (words.size() > 3 && sscanf_s(words[3].c_str(), "%d", &val3))
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
				// For both MFLO/MFHI and JR instructions
				rs = val1;
				rd = val1;
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
					// Special case here. LUI wants the reg in RT, BLEZ needs it in RS.
					if (opcode == "lui")
					{
						// $rt, imm
						rt = val1;
						imm = val2;
					}
					else
					{
						// $rs, imm
						rs = val1;
						imm = val2;
					}
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
	{ "mfhi"  , { 0,  16, R_TYPE } } ,		// TESTED
	//{ "mthi"  , { 0,  17, R_TYPE } } ,		// WON'T IMPLEMENT
	{ "mflo"  , { 0,  18, R_TYPE } } ,		// TESTED
	//{ "mtlo"  , { 0,  19, R_TYPE } } ,		// WON'T IMPLEMENT
	{ "mult"  , { 0,  24, R_TYPE } } ,		// TESTED
	{ "multu" , { 0,  25, R_TYPE } } ,		// TESTED
	//{ "div"   , { 0,  26, R_TYPE } } ,		// WON'T IMPLEMENT
	//{ "divu"  , { 0,  27, R_TYPE } } ,		// WON'T IMPLEMENT
	{ "add"   , { 0,  32, R_TYPE } } ,		// TESTED
	{ "addu"  , { 0,  33, R_TYPE } } ,		// TESTED
	{ "sub"   , { 0,  34, R_TYPE } } ,		// TESTED
	{ "subu"  , { 0,  35, R_TYPE } } ,		// TESTED
	{ "and"   , { 0,  36, R_TYPE } } ,		// TESTED
	{ "or"    , { 0,  37, R_TYPE } } ,		// TESTED
	{ "xor"   , { 0,  38, R_TYPE } } ,		// TESTED
	{ "nor"   , { 0,  39, R_TYPE } } ,		// TESTED
	{ "slt"   , { 0,  42, R_TYPE } } ,		// TESTED
	{ "sltu"  , { 0,  43, R_TYPE } } ,		// TESTED
	{ "j"	  , { 2,  0,  J_TYPE } } ,		// TESTED
	{ "jal"	  , { 3,  0,  J_TYPE } } ,		// TESTED
	{ "beq"   , { 4,  0,  I_TYPE } } ,		// TESTED
	{ "bne"   , { 5,  0,  I_TYPE } } ,		// TESTED
	{ "blez"  , { 6,  0,  I_TYPE } } ,		// TESTED
	{ "bgtz"  , { 7,  0,  I_TYPE } } ,		// TESTED
	{ "addi"  , { 8,  0,  I_TYPE } } ,		// TESTED
	{ "addiu" , { 9,  0,  I_TYPE } } ,		// TESTED
	{ "slti"  , { 10, 0,  I_TYPE } } ,		// TESTED
	{ "sltiu" , { 11, 0,  I_TYPE } } ,		// TESTED
	{ "andi"  , { 12, 0,  I_TYPE } } ,		// TESTED
	{ "ori"   , { 13, 0,  I_TYPE } } ,		// TESTED
	{ "xori"  , { 14, 0,  I_TYPE } } ,		// TESTED
	{ "lui"   , { 15, 0,  I_TYPE } } ,		// TESTED
	{ "lb"    , { 32, 0,  I_TYPE } } ,		// TESTED
	{ "lh"    , { 33, 0,  I_TYPE } } ,		// TESTED
	//{ "lwl"   , { 34, 0,  I_TYPE } } ,		// WON'T IMPLEMENT
	{ "lw"    , { 35, 0,  I_TYPE } } ,		// TESTED
	{ "lbu"   , { 36, 0,  I_TYPE } } ,		// TESTED
	{ "lhu"   , { 37, 0,  I_TYPE } } ,		// TESTED
	//{ "lwr"   , { 38, 0,  I_TYPE } } ,		// WON'T IMPLEMENT
	{ "sb"    , { 40, 0,  I_TYPE } } ,		// TESTED
	{ "sh"    , { 41, 0,  I_TYPE } } ,		// TESTED
	//{ "swl"   , { 42, 0,  I_TYPE } } ,		// WON'T IMPLEMENT
	{ "sw"    , { 43, 0,  I_TYPE } } ,		// TESTED
	//{ "swr"   , { 46, 0,  I_TYPE } } ,		// WON'T IMPLEMENT
	{ "brk"   , { 62, 0,  X_TYPE } },		// TESTED
	{ "hlt"   , { 63, 0,  X_TYPE } },		// TESTED
	});



const std::array<std::string, 32> Assembler::reg_names({
	"r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
});
