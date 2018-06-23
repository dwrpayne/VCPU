#pragma once
#include "Component.h"
#include "NandGate.h"
#include "Inverter.h"

class DFlipFlop : public Component
{
public:
	void Connect(const Wire& d, const Wire& e);
	void Update();

	const Wire& Q() { return nandR.Out(); }
	const Wire& NotQ() { return nandS.Out(); }

private:
	Inverter invD;
	NandGate nandD;
	NandGate nandDinv;
	NandGate nandR;
	NandGate nandS;
};

// This flip flop starts at 0, sets to 1 like normal, but then can't ever be reset to 0.
class DFlipFlopSticky : public Component
{
public:
	void Connect(const Wire& d, const Wire& e);
	void Update();

	const Wire& Q() { return bit.Q(); }
	const Wire& NotQ() { return bit.NotQ(); }

private:
	AndGate enable;
	DFlipFlop bit;
};