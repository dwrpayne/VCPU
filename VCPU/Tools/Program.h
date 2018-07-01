#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>


std::string TrimWhitespace(const std::string& s);

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

	std::string to_string() const 
	{
		std::stringstream ss;
		ss << std::left << std::setw(15) << mLabel << std::setw(6) << mLineNum << std::setw(25) << TrimWhitespace(mSource);
		return ss.str();
	}
};

class Program
{
public:
	Program();
	unsigned int AddSourceLine(const std::string& label, std::string source, const std::string& comment);
	void AddInstruction(unsigned int source_line, const std::string& text);
	void AddTextField(const std::string& label, int size, const std::vector<unsigned char>& bytes);

	const Instruction* GetInstruction(unsigned int addr) const;	
	const CodeLine* GetLine(unsigned int addr) const;

	const std::string GetSourceLine(unsigned int addr) const;
	const std::string GetAssembledLine(unsigned int addr) const;

	std::vector<Instruction>& Instructions() { return mInstructions; }
	const std::vector<Instruction>& Instructions() const { return mInstructions; }

	const std::vector<unsigned char> TextBytes() const { return mTextBytes; }
	unsigned int GetTextStartAddr() const { return mTextStartAddr; }
	
	void ConvertLabels();
	void ReplaceLabel(const std::string & label, unsigned int addr);

private:
	void AddLabel(const std::string& label, unsigned int addr);

	std::vector<CodeLine> mSourceLines;
	std::vector<Instruction> mInstructions;

	unsigned int mTextStartAddr;
	std::vector<unsigned char> mTextBytes;
	std::map<std::string, unsigned int> mLabelAddrNum;
};