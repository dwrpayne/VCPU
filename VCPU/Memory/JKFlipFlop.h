#pragma once
#include "Component.h"
#include "AndGate.h"
#include "SRLatch.h"


class JKFlipFlop : public Component
{
public:
	void Connect(const Wire& j, const Wire& k);
	void Update();

	const Wire& Q() const { return latch.Q(); }
	const Wire& NotQ() const { return latch.NotQ(); }

private:
	AndGate andj;
	AndGate andk;
	SRLatch latch;
};

