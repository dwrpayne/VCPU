#pragma once

#include "Component.h"
#include "Bundle.h"
#include "MuxBundle.h"
#include "Matcher.h"
#include "OrGate.h"
#include "NorGate.h"


class Interlock : public Component
{
public:
	static const int ADDR = 5;
	typedef Bundle<ADDR> RegBundle;

	void Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2, 
		const RegBundle& writingRegIDEX, const Wire& loadopIDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM, 
		const Bundle<6>& opcodeIF);
	void Update();

	const Wire& Bubble() { return bubble.Out(); }
	const Wire& Freeze() { return freeze.Out(); }
	const Wire& FreezeInv() { return freezeInv.Out(); }
	const Wire& FreezeOrBubbleInv() { return freezeOrBubbleInv.Out(); }

private:
	OrGate branchorloadexmem;
	NonZeroMatcher<ADDR> idexMatcher;
	NonZeroMatcher<ADDR> exmemMatcher;
	NorGateN<3> branchopnor;
	AndGate branchopand;
	AndGate branchExMemMatch;
	OrGate bubble;
	Inverter bubbleInv;
	OrGate freeze;
	Inverter freezeInv;
	NorGate freezeOrBubbleInv;
};
