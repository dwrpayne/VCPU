#pragma once
#include "Component.h"
#include "NandGate.h"
#include "SRLatch.h"
#include "NorGate.h"


class JKFlipFlop : public Component
{
public:
	void Connect(const Wire& j, const Wire& k);
	void Update();

	const Wire& Q() { return latch.Q(); }
	const Wire& NotQ() { return latch.NotQ(); }

private:
	AndGate andj;
	AndGate andk;
	SRLatch latch;
};

