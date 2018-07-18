#pragma once
#include "Component.h"
#include "AndGate.h"
#include "NorGate.h"
#include "Inverter.h"

class DFlipFlop : public Component
{
public:
	void Connect(const Wire& d, const Wire& e);
	void Update()
	{
		invD.Update();
		andD.Update();
		andDinv.Update();
		norD.Update();
		norDinv.Update();
		norD.Update();
	}

	const Wire& Q() const { return norDinv.Out(); }
	const Wire& NotQ() const { return norD.Out(); }

private:
	Inverter invD;
	AndGate andDinv;
	AndGate andD;
	NorGate norDinv;
	NorGate norD;
};
