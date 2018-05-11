#pragma once
#include <array>

#include "Wire.h"
#include "Bundle.h"
#include "Component.h"


class AndGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return out; }

private:
	const Wire* in1;
	const Wire* in2;
	Wire out;
};
