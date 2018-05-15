#include "Assembler.h"
#include <string>
#include <vector>
#include <fstream>


Assembler::Assembler(const std::string& filename)
{
	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line))
	{
		ParseLine(line);
	}
}

std::vector<std::string> split(const char *str)
{
	std::vector<std::string> result;
	do
	{
		const char *begin = str;
		while (*str != ' ' && *str != '\t' && *str)
			str++;

		result.push_back(std::string(begin, str));
	} while (0 != *str++);

	return result;
}

// Expects one of three basic format. Nothing fancy is supported, just raw numbers for registers. No comments.
// Space separat
// R: OP RS RT RD SH
// I: OP RS RT IMM
// J: OP ADDR
bool Assembler::ParseLine(const std::string& line)
{
	std::vector<std::string> words = split(line.c_str());
	std::string opcode = words[0];

	if (words.size() == 2)
	{
		program.push_back({ opcode.c_str(), std::stoi(words[1]) });
	}
	else if (words.size() == 4)
	{
		program.push_back({ opcode.c_str(), std::stoi(words[1]), std::stoi(words[2]), std::stoi(words[3]) });
	}
	else if (words.size() == 5)
	{
		program.push_back({ opcode.c_str(), std::stoi(words[1]), std::stoi(words[2]), std::stoi(words[3]), std::stoi(words[4]) });
	}
	else
	{
		std::cout << "ERROR: Couldn't parse line " << line << std::endl;
		return false;
	}
	return true;
}

