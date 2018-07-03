#pragma once
#include "Component.h"
#include "AndGate.h"
#include "OrGate.h"
#include "Inverter.h"

class Wire;

class XNorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() const { return orGate.Out(); }

private:
	AndGate and;
	Inverter aInv, bInv;
	AndGate invAnd;
	OrGate orGate;
};

