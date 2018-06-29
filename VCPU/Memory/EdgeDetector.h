#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "DFlipFlop.h"
#include "AndGate.h"

// EdgeDetector pulses exactly once on input wire transition from 0->1 (Rise) and 1->0 (Fall)
class EdgeDetector : public Component
{
public:
	void Connect(const Wire& in);
	void Update();

	const Wire& Rise() const { return rise.Out(); }
	const Wire& Fall() const { return fall.Out(); }

private:
	DFlipFlop now;
	DFlipFlop last;
	AndGate rise;
	AndGate fall;
};
