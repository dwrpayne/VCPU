#pragma once

#include "Component.h"
#include "Wire.h"
#include "OrGate.h"
#include "AndGate.h"
#include "XorGate.h"


class FullAdder : public Component
{
public:
	void Connect(const Wire& a, const Wire& b, const Wire& c);
	void Update();

	const Wire& S() { return xorOut.Out(); }
	const Wire& Cout() { return orGate.Out(); }

private:
	XorGate xorIn;
	AndGate andIn;
	XorGate xorOut;
	AndGate andOut;
	OrGate orGate;
};