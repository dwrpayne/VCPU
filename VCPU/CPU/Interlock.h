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
		const RegBundle& writingReg, const Wire& loadop, const Bundle<6>& opcodeIF);
	void Update();

	const Wire& Bubble() { return bubble.Out(); }
	const Wire& Freeze() { return freeze.Out(); }
	const Wire& FreezeInv() { return freezeInv.Out(); }
	const Wire& FreezeOrBubbleInv() { return freezeOrBubbleInv.Out(); }

private:
	Matcher<ADDR> r1match;
	Matcher<ADDR> r2match;
	OrGateN<ADDR> regNonZero;
	OrGate regmatch;
	NorGateN<3> branchopnor;
	AndGate branchopand;
	OrGate loadorbranch;
	AndGateN<3> bubble;
	Inverter bubbleInv;
	OrGate freeze;
	Inverter freezeInv;
	NorGate freezeOrBubbleInv;
};
