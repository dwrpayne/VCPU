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
