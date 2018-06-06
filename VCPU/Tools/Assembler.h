#pragma once
#include <vector>
#include <map>
#include <regex>

enum InstType
{
	R_TYPE,
	I_TYPE,
	J_TYPE,
	X_TYPE,
};

class Assembler
{
public:
	Assembler(const std::string& filename);
	void ParseSource();

	const std::string GetSourceLine(unsigned int line) const;
	const std::string GetAssembledLine(unsigned int line) const;
	std::vector<unsigned int>& GetBinary() { return mBinary; }
	const std::string GetRegName(unsigned int reg) const;
		
private:
	std::vector<std::string> ParseLine(const std::string& line);
	unsigned int GetMachineLanguage(const std::string & line);

	std::string mSourceFilename;

	std::vector<std::string> mSource;
	std::vector<std::string> mComments;
	std::vector<unsigned int> mSourceLine;
	std::vector<std::string> mAssembled;
	std::vector<unsigned int> mBinary;

	static const std::vector<std::pair<std::regex, std::string>> instructionMnemonics;
	static const std::map<std::string, std::tuple<unsigned char, unsigned char, InstType>> opcodeInfo;
	static const std::array<std::string, 32> reg_names;
};

