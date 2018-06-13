#include "Program.h"
#include <sstream>
#include <iomanip>

unsigned int Program::AddSourceLine(std::string label, std::string source, std::string comment)
{
	// There's no line 0, start at 1
	mSourceLines.push_back(CodeLine(label, source, comment, mSourceLines.size() + 1));
	return mSourceLines.size() - 1;
}

void Program::AddInstruction(unsigned int source_line, std::string text, unsigned int binary)
{
	mInstructions.push_back(Instruction(text, binary, source_line));
}

const Instruction * Program::GetIns(unsigned int addr) const
{
	if (addr >= mInstructions.size())
	{
		return nullptr;
	}
	return &mInstructions[addr];
}

const CodeLine * Program::GetLine(unsigned int addr) const
{
	if (const Instruction* ins = GetIns(addr))
	{
		if (ins->mSourceLineNum < mSourceLines.size())
		{
			return &mSourceLines[ins->mSourceLineNum];
		}
	}
	return nullptr;
}

const std::string Program::GetAssembledLine(unsigned int line)  const
{
	if (auto ins = GetIns(line))
	{
		return ins->mText;
	}
	return "";
}

const std::string Program::GetSourceLine(unsigned int line) const
{
	const CodeLine* source = GetLine(line);
	if (!source)
	{
		return "";
	}
	std::stringstream ss;
	ss << std::left << std::setw(3) << source->mLineNum << std::setw(25) << source->mSource << source->mComment;
	return ss.str();
}