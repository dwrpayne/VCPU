#pragma once

#include "OrGate.h"
#include "Inverter.h"
#include "Component.h"
#include "Bundle.h"

class Wire;

class NorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() const { return inv.Out(); }

private:
	OrGate or;
	Inverter inv;
};

template <unsigned int N>
class NorGateN : public Component
{
public:
	void Connect(const Bundle<N>& bundle);
	void Update();

	const Wire& Out() const { return inv.Out(); }
	
private:
	OrGateN<N> ors;
	Inverter inv;
};

template<unsigned int N>
inline void NorGateN<N>::Connect(const Bundle<N>& bundle)
{
	ors.Connect(bundle);
	inv.Connect(ors.Out());
}

template<unsigned int N>
inline void NorGateN<N>::Update()
{
	ors.Update();
	inv.Update();
}
