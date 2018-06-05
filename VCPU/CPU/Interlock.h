#pragma once

#include "Component.h"
#include "Bundle.h"
#include "MuxBundle.h"
#include "Matcher.h"
#include "OrGate.h"

class Interlock : public Component
{
public:
	static const int ADDR = 5;
	typedef Bundle<ADDR> RegBundle;

	void Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2, 
		const RegBundle& loadingReg, const Wire& loadop);
	void Update();

	const Wire& Bubble() { return bubble.Out(); }
	const Wire& BubbleInv() { return bubbleInv.Out(); }

private:
	Matcher<ADDR> r1match;
	Matcher<ADDR> r2match;
	OrGate regmatch;
	AndGate stallRAW;
	OrGateN<3> bubble;
	Inverter bubbleInv;
};
