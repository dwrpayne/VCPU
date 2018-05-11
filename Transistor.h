#pragma once

#include "Wire.h"
#include "Component.h"

class Transistor : public Component
{
public:
	Transistor(const Wire& b, const Wire& c) { Connect(b, c); }
	void Connect(const Wire& b, const Wire& c);
	void Update();

	const Wire& GetOutput() { return emitter; }

private:
	const Wire* base;
	const Wire* collector;
	Wire emitter;
};