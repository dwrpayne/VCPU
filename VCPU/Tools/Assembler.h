#pragma once
#include <vector>
#include "Instruction.h"

class Assembler
{
public:
	Assembler(const std::string& filename);
	bool ParseLine(const std::string& line);

	std::vector<Instruction>& GetProgram() { return program; }
		
private:
	std::vector<Instruction> program;
};

