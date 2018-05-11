#pragma once

#include "Wire.h"
#include "Component.h"

class OrGate : public Component
{
public:
	OrGate(const Wire& a, const Wire& b) { Connect(a, b); }
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return out; }

private:
	const Wire* in1;
	const Wire* in2;
	Wire out;
};