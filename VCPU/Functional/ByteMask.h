#pragma once
#include <array>
#include "Component.h"
#include "Bundle.h"
#include "MuxBundle.h"
#include "MultiGate.h"

// ByteMask creates a 32-bit mask of the specified byte, half-word, or word

class ByteMask : public Component
{
public:
	typedef Bundle<32> Word;
	void Connect(const Bundle<2> byteindex, const Wire& byte, const Wire& half, const Wire& word)
	{
		bytemasker.Connect({ Word(0xffU), Word(0xff00U), Word(0xff0000U), Word(0xff000000U) }, byteindex);
		halfmasker.Connect({ Word(0xffffU), Word(0xffff0000U) }, byteindex[1]);
		masker.Connect({ Word::ON, bytemasker.Out(), halfmasker.Out(), Word::ON }, { &byte, &half });
		write.Connect(masker.Out(), Word(word));
	}
	void Update()
	{
		bytemasker.Update();
		halfmasker.Update();
		masker.Update();
		write.Update();
	}
	const Word& Mask() const { return write.Out(); }

private:
	MuxBundle<32, 4> bytemasker;
	MuxBundle<32, 2> halfmasker;
	MuxBundle<32, 4> masker;
	MultiGate<AndGate, 32> write;
};

class WordMasker : public Component
{
public:
	void Connect(const Bundle<2> byteindex, const Bundle<32>& dataword, const Wire& byte, const Wire& half, const Wire& word)
	{
		mask.Connect(byteindex, byte, half, word);
		wordShifter.Connect(dataword, byteindex.ShiftZeroExtend<5>(3));
		maskedDataWord.Connect(wordShifter.Out(), mask.Mask());
	}
	void Update()
	{
		mask.Update();
		wordShifter.Update();
		maskedDataWord.Update();
	}
	const Bundle<32>& WordMask() const { return mask.Mask(); }
	const Bundle<32>& Word() const { return maskedDataWord.Out(); }

private:
	ByteMask mask;
	LeftShifter<32> wordShifter;
	MultiGate<AndGate, 32> maskedDataWord;
};