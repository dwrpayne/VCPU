#pragma once
#include <array>
#include "Component.h"
#include "Bundle.h"
#include "SubWordSelector.h"

// ByteExtractor pulls the specified byte, half-word, or word from the provided 32-bit word

class ByteExtractor : public Component
{
public:
	typedef Bundle<32> Word;
	void Connect(const Word word, const Bundle<2>& byteindex, const Wire& signextend, const Wire& byte, const Wire& half)
	{
		byteSelect.Connect(word, byteindex, signextend);
		halfWordSelect.Connect(word, Bundle<1>(byteindex[1]), signextend);
		outMux.Connect({ word, byteSelect.Out(), halfWordSelect.Out(), word },	{ &byte, &half });

	}
	void Update()
	{
		byteSelect.Update();
		halfWordSelect.Update();
		outMux.Update();
	}
	const Word& Out() const { return outMux.Out(); }

private:
	SubWordSelector<32> byteSelect;
	SubWordSelector<32, 16> halfWordSelect;
	MuxBundle<32, 4> outMux;
};