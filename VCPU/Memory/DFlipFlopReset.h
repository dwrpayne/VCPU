#pragma once
#include "Component.h"
#include "NandGate.h"
#include "Inverter.h"

class DFlipFlopReset : public Component
{
public:
	void Connect(const Wire& d, const Wire& e, const Wire& reset);
	void Update();

	const Wire& Q() { return nandR.Out(); }
	const Wire& NotQ() { return nandS.Out(); }

private:
	Inverter invD;
	Inverter invR;
	NandGate nandD;
	NandGate nandDinv;
	NandGate nandR;
	NandGateN<3> nandS;
};

