#pragma once

#include "Component.h"
#include "Wire.h"
#include "Inverter.h"
#include "AndGate.h"
#include "OrGate.h"


class OverflowDetector : public Component
{
public:
	void Connect(const Wire& a, const Wire& b, const Wire& sum, const Wire& enable);
	void Update();

	const Wire& Out() { return overflow.Out(); }

	Inverter invA;
	Inverter invB;
	Inverter invSum;
	AndGateN<4> overflowPos;
	AndGateN<4> overflowNeg;
	OrGate overflow;
};

inline void OverflowDetector::Connect(const Wire & a, const Wire & b, const Wire & sum, const Wire & enable)
{
	invA.Connect(a);
	invB.Connect(b);
	invSum.Connect(sum);
	overflowPos.Connect({ &a, &b, &invSum.Out(), &enable });
	overflowNeg.Connect({ &invA.Out(), &invB.Out(), &sum, &enable });
	overflow.Connect(overflowNeg.Out(), overflowPos.Out());
}

inline void OverflowDetector::Update()
{
	invA.Update();
	invB.Update();
	invSum.Update();
	overflowPos.Update();
	overflowNeg.Update();
	overflow.Update();
}

