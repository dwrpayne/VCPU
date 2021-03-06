#include "Program.h"
#include "CPU/Addresses.h"
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <algorithm>
#include <regex>

Program::Program()
	: mTextStartAddr(USER_TEXT_START)
{
}

unsigned int Program::AddSourceLine(const std::string& label, std::string source, const std::string& comment)
{
	// Insert the line.
	unsigned int line_num = mSourceLines.size();
	auto codeline = CodeLine(label, TrimWhitespace(source), comment, line_num);
	codeline.mFirstInstructionNum = mInstructions.size();
	mSourceLines.push_back(codeline);

	if (label.size() > 0)
	{
		AddLabel(label, codeline.mFirstInstructionNum * 4);
	}

	return line_num;
}

void Program::AddInstruction(unsigned int source_line, const std::string& text)
{
	mSourceLines[source_line].mFirstInstructionNum = mInstructions.size();
	mInstructions.push_back(Instruction(TrimWhitespace(text), 0, source_line));
}

void Program::AddTextField(const std::string & label, int size, const std::vector<unsigned char>& bytes)
{
	assert(label.size() > 0);
	AddLabel(label, mTextBytes.size() + mTextStartAddr);
	mTextBytes.insert(mTextBytes.end(), bytes.begin(), bytes.end());
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
	return source->to_string();
}

const std::string Program::GetAssembledLine(unsigned int addr)  const
{
	if (auto ins = GetInstruction(addr))
	{
		return ins->mText;
	}
	return "";
}

void Program::AddLabel(const std::string & label, unsigned int addr)
{
	assert(mLabelAddrNum.count(label) == 0 && "Found two instances of the label.");
	mLabelAddrNum[label] = addr;
}

// Turns all the labels in the instructions into actual number offsets
// This knows that J instructions are absolute address and B instructions are relative
// Perhaps that is an Assembler piece of knowledge.
void Program::ConvertLabels()
{
	std::vector<std::pair<std::string, unsigned int>> items(mLabelAddrNum.begin(), mLabelAddrNum.end());
	std::sort(items.begin(), items.end(), [](const std::pair<std::string, unsigned int>& a, const std::pair<std::string, unsigned int>& b)
	{
		return a.first.size() > b.first.size();
	});
	for (const auto&[label, addr] : items)
	{
		ReplaceLabel(label, addr);
	}
}

void Program::ReplaceLabel(const std::string& label, unsigned int addr)
{
	for (unsigned int i = 0; i < mInstructions.size(); ++i)
	{
		auto label_pos = mInstructions[i].mText.find(label);
		if (label_pos != std::string::npos)
		{
			auto first_alpha = mInstructions[i].mText.find_first_not_of(" \t\r\n");
			int mem_addr = addr;
			if (mInstructions[i].mText[first_alpha] == 'b')
			{
				mem_addr -= 4 * (i + 1);
			}
			else
			{
				//assert((mInstructions[i].mText[first_alpha] == 'j' || mInstructions[i].mText[first_alpha] == 'l') && "Labels must be on J or B or L instructions");
			}
			mInstructions[i].mText.replace(label_pos, label.size(), std::to_string(mem_addr));
		}
	}
}

std::string TrimWhitespace(const std::string & s)
{
	// Strip begin and end whitespace, and collapse multiple
	std::string line = std::regex_replace(s, std::regex("^\\s*"), "");
	line = std::regex_replace(line, std::regex("\\s*$"), "");
	line = std::regex_replace(line, std::regex("\\s+"), " ");

	// Replace the first space with a fixed tab.
	auto first_space = line.find_first_of(' ');
	if (first_space != std::string::npos)
	{
		line.insert(first_space, 6 - first_space, ' ');
	}
	return line;
}
