#pragma once

#include "Component.h"
#include "NorGate.h"

class Wire;

class SRLatch :	public Component
{
public:
	void Connect(const Wire& s, const Wire& r);
	void Update();

	const Wire& Q() const { return norr.Out(); }
	const Wire& NotQ() const { return nors.Out(); }

private:
	NorGate norr;
	NorGate nors;
};

