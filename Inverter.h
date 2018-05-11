#pragma once
#include "Wire.h"
#include "Component.h"

class Inverter : public Component
{
public:
	Inverter(const Wire& a) { Connect(a); }
	void Connect(const Wire& a);
	void Update();

	const Wire& Out() { return out; }

private:
	const Wire* in;
	Wire out;
};

