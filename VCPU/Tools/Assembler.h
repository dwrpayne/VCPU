#pragma once
#include <vector>
#include <map>

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
	std::vector<unsigned int>& GetBinary() { return mBinary; }
		
private:
	unsigned int ParseLine(const std::string& line);

	std::string mSourceFilename;

	std::vector<std::string> mSource;
	std::vector<unsigned int> mBinary;

	static const std::map<std::string, std::tuple<unsigned char, unsigned char, InstType>> opcodeInfo;
};

