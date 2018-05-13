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

private:
	NorGate norr;
	NorGate nors;
};


class SRLatchPreset : public Component
{
public:
	void Connect(const Wire& s, const Wire& r, const Wire& preset, const Wire& clear);
	void Update();

	const Wire& Q() { return nands.Out(); }
	const Wire& NotQ() { return nandr.Out(); }

private:
	Inverter sInv;
	Inverter rInv;
	Inverter presetInv;
	Inverter clearInv;
	NandGateN<3> nandr;
	NandGateN<3> nands;
};

