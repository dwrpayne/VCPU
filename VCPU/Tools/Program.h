#pragma once

#include <vector>
#include <string>
#include <map>

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
	unsigned int mFirstInstructionNum;
};

class Program
{
public:
	Program();
	unsigned int AddSourceLine(const std::string& label, std::string source, const std::string& comment);
	void AddInstruction(unsigned int source_line, const std::string& text);
	const Instruction* GetInstruction(unsigned int addr) const;	
	const CodeLine* GetLine(unsigned int addr) const;

	const std::string GetSourceLine(unsigned int addr) const;
	const std::string GetAssembledLine(unsigned int addr) const;

	std::vector<Instruction>& Instructions() { return mInstructions; }
	const std::vector<Instruction>& Instructions() const { return mInstructions; }
	
	void ConvertLabels();
	void ReplaceLabel(const std::string & label, unsigned int addr);

private:
	std::string TrimWhitespace(const std::string& s);
	std::vector<CodeLine> mSourceLines;
	std::vector<Instruction> mInstructions;

	std::map<std::string, unsigned int> mLabelAddrNum;
};