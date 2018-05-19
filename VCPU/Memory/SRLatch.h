#pragma once

#include "Component.h"
#include "NorGate.h"
#include "NandGate.h"

class SRLatch :	public Component
{
public:
	void Connect(const Wire& s, const Wire& r);
	void Update();

	const Wire& Q() { return norr.Out(); }
	const Wire& NotQ() { return nors.Out(); }

	virtual int Cost() const
	{
		return norr.Cost() + nors.Cost();
	}

private:
	NorGate norr;
	NorGate nors;
};

