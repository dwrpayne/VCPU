#pragma once

#include <vector>
#include <string>

class Instruction
{
public:
	Instruction(std::string text, unsigned int binary, unsigned int sourceline)
		: mText(text)
		, mBinary(binary)
		, mSourceLineNum(sourceline)
	{}
	std::string mText;
	unsigned int mBinary;
	unsigned int mSourceLineNum;
};

class CodeLine
{
public:
	CodeLine(std::string label, std::string source, std::string comment, unsigned int linenum)
		: mLabel(label)
		, mSource(source)
		, mComment(comment)
		, mLineNum(linenum)
	{}
	std::string mLabel;
	std::string mSource;
	std::string mComment;
	unsigned int mLineNum;
};

class Program
{
public:
	Program() {}
	unsigned int AddSourceLine(std::string label, std::string source, std::string comment);
	void AddInstruction(unsigned int source_line, std::string text, unsigned int binary);
	const Instruction* GetIns(unsigned int addr) const;
	const CodeLine* GetLine(unsigned int addr) const;

	const std::string GetSourceLine(unsigned int line) const;
	const std::string GetAssembledLine(unsigned int line) const;

	const std::vector<Instruction>& Instructions() const { return mInstructions; }

private:
	std::vector<CodeLine> mSourceLines;
	std::vector<Instruction> mInstructions;
};