#pragma once
#include <array>

#include "Wire.h"
#include "Bundle.h"
#include "Component.h"


class AndGate : public Component
{
public:
	AndGate();
	AndGate(const Wire& a, const Wire& b);
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out;

private:
	const Wire* in1;
	const Wire* in2;
	Wire out;
};
