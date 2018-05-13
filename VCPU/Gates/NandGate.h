#pragma once

#include "Wire.h"
#include "AndGate.h"
#include "Inverter.h"
#include "Component.h"

class NandGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return inv.Out(); }

private:
	AndGate and;
	Inverter inv;
};

template <unsigned int N>
class NandGateN : public Component
{
public:
	void Connect(const Bundle<N>& bundle);
	void Update();

	const Wire& Out() { return inv.Out(); }

private:
	AndGateN<N> ands;
	Inverter inv;
};

template<unsigned int N>
inline void NandGateN<N>::Connect(const Bundle<N>& bundle)
{
	ands.Connect(bundle);
	inv.Connect(ands.Out());
}

template<unsigned int N>
inline void NandGateN<N>::Update()
{
	ands.Update();
	inv.Update();
}
