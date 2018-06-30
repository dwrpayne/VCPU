#pragma once
#include <vector>
#include <map>
#include <regex>

class Program;

enum InstType
{
	R_TYPE,
	I_TYPE,
	J_TYPE,
	X_TYPE,
};

enum Section
{
	CODE,
	TEXT,
	SECTION_NONE
};


class Assembler
{
public:
	Assembler();
	const Program* Assemble(const std::string& filename);

	const std::string GetRegName(unsigned int reg) const;
		
private:
	std::vector<std::string> GetInstructionsForLine(const std::string& line);
	unsigned int GetMachineLanguage(const std::string & line);

	void ParseFile(const std::string & filename, Program * program);
	void ParseSourceLine(const std::string &line, Program * program);
	void IncludeLib(const std::string& filename);

	std::vector<std::string> mIncludeFiles;
		
	static const std::vector<std::pair<std::regex, std::string>> instructionMnemonics;
	static const std::map<std::string, std::tuple<unsigned char, unsigned char, InstType>> opcodeInfo;
	static const std::array<std::string, 32> reg_names;

	Section mCurSection;

	Program* pProgram;
};

