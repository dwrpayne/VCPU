#pragma once

#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "Component.h"

class Wire;

class XorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() const { return and.Out(); }

private:
	NandGate nand;
	OrGate or;
	AndGate and;
};

