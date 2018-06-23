#pragma once
#include <array>
#include "Component.h"
#include "Bundle.h"
#include "Multiplexer.h"

// ByteExtractor pulls the specified byte, half-word, or word from the provided 32-bit word

class ByteExtractor : public Component
{
public:
	typedef Bundle<32> Word;
	void Connect(const Bundle<2> byteindex, const Wire& bytewrite, const Wire& halfwrite, const Wire& wordwrite)
	{
		bytemasker.Connect({ Word(0xffU), Word(0xff00U), Word(0xff0000U), Word(0xff000000U) }, byteindex);
		halfmasker.Connect({ Word(0xffffU), Word(0xffff0000U) }, byteindex[1]);
		masker.Connect({ Word::ON, bytemasker.Out(), halfmasker.Out(), Word::ON }, { &bytewrite, &halfwrite });
		write.Connect(masker.Out(), Word(wordwrite));
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