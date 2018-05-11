#pragma once

#include "Wire.h"
#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "Component.h"

class XorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return and.Out(); }

private:
	NandGate nand;
	OrGate or;
	AndGate and;
};
