#pragma once

#include "Component.h"
#include "NorGate.h"

class SRLatch :	public Component
{
public:
	void Connect(const Wire& s, const Wire& r);
	void Update();

	const Wire& Q() { return norr.Out(); }
	const Wire& NotQ() { return nors.Out(); }

private:
	NorGate norr;
	NorGate nors;
};

