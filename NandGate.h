#pragma once

#include "Wire.h"
#include "AndGate.h"
#include "Inverter.h"
#include "Component.h"

class NandGate : public Component
{
public:
	NandGate(const Wire& a, const Wire& b);
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return inv.Out(); }

private:
	AndGate and;
	Inverter inv;
};
