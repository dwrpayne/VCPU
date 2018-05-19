#pragma once
#include "Component.h"
#include "Wire.h"
#include "AndGate.h"
#include "OrGate.h"
#include "Inverter.h"

class XNorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return orGate.Out(); }

	virtual int Cost() const
	{
		return and.Cost() + aInv.Cost() + bInv.Cost() + invAnd.Cost() + orGate.Cost();
	}

private:
	AndGate and;
	Inverter aInv, bInv;
	AndGate invAnd;
	OrGate orGate;
};

