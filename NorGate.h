#pragma once

#include "Wire.h"
#include "OrGate.h"
#include "Inverter.h"
#include "Component.h"

class NorGate : public Component
{
public:
	NorGate(const Wire& a, const Wire& b);
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return inv.Out(); }

private:
	OrGate or;
	Inverter inv;
};