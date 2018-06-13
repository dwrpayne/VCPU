#include "Program.h"
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <algorithm>
#include <regex>

unsigned int Program::AddSourceLine(const std::string& label, std::string source, const std::string& comment)
{
	// Strip begin and end whitespace, and collapse multiple
	source = std::regex_replace(source, std::regex("^\\s*"), "");
	source = std::regex_replace(source, std::regex("\\s*$"), "");
	source = std::regex_replace(source, std::regex("\\s+"), " ");

	// Replace the first space with a fixed tab.
	auto first_space = source.find_first_of(' ');
	if (first_space != std::string::npos)
	{
		source.insert(first_space, 6 - first_space, ' ');
	}

	// Insert the line.
	unsigned int line_num = mSourceLines.size();
	auto codeline = CodeLine(label, source, comment, line_num);
	codeline.mFirstInstructionNum = mInstructions.size();
	mSourceLines.push_back(codeline);

	if (label.size() > 0)
	{
		assert(mLabelAddrNum.count(label) == 0 && "Found two instances of the label.");
		mLabelAddrNum[label] = codeline.mFirstInstructionNum;
	}

	return line_num;
}

void Program::AddInstruction(unsigned int source_line, const std::string& text)
{
	mSourceLines[source_line].mFirstInstructionNum = mInstructions.size();
	mInstructions.push_back(Instruction(text, 0, source_line));
}

const Instruction * Program::GetInstruction(unsigned int addr) const
{
	if (addr >= mInstructions.size())
	{
		return nullptr;
	}
	return &mInstructions[addr];
}

const CodeLine * Program::GetLine(unsigned int addr) const
{
	if (const Instruction* ins = GetInstruction(addr))
	{
		if (ins->mSourceLineNum < mSourceLines.size())
		{
			return &mSourceLines[ins->mSourceLineNum];
		}
	}
	return nullptr;
}

const std::string Program::GetSourceLine(unsigned int addr) const
{
	const CodeLine* source = GetLine(addr);
	if (!source)
	{
		return "";
	}
	std::stringstream ss;
	ss << std::left << std::setw(3) << source->mLineNum << std::setw(25) << source->mSource << source->mComment;
	return ss.str();
}

const std::string Program::GetAssembledLine(unsigned int addr)  const
{
	if (auto ins = GetInstruction(addr))
	{
		return ins->mText;
	}
	return "";
}

// Turns all the labels in the instructions into actual number offsets
// This knows that J instructions are absolute address and B instructions are relative
// Perhaps that is an Assembler piece of knowledge.
void Program::ConvertLabels()
{
	for (const auto&[label, addr] : mLabelAddrNum)
	{
		for (unsigned int i = 0; i < mInstructions.size(); ++i)
		{
			auto label_pos = mInstructions[i].mText.find(label);
			if (label_pos != std::string::npos)
			{
				auto first_alpha = mInstructions[i].mText.find_first_not_of(" \t\r\n");
				int mem_addr = addr * 4;
				if (mInstructions[i].mText[first_alpha] == 'b')
				{
					mem_addr -= 4 * (i + 1);
				}
				else
				{
					assert(mInstructions[i].mText[first_alpha] == 'j' && "Labels must be on J or B instructions");
				}
				mInstructions[i].mText.replace(label_pos, label.size(), std::to_string(mem_addr));
			}			
		}
	}
}
