#pragma once
#include <array>
#include "Wire.h"
#include "Component.h"
#include "Bundle.h"

class OrGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return out; }

	virtual int Cost() const
	{
		return 1;
	}

private:
	const Wire* in1;
	const Wire* in2;
	Wire out;
};

template <unsigned int N>
class OrGateN : public Component
{
public:
	void Connect(const Bundle<N>& wires);
	void Update();

	const Wire& Out() { return ors[N - 2].Out(); }

	virtual int Cost() const
	{
		return ArrayCost(ors);
	}

private:
	std::array<OrGate, N - 1> ors;
};

template<unsigned int N>
inline void OrGateN<N>::Connect(const Bundle<N>& wires)
{
	ors[0].Connect(wires[0], wires[1]);
	for (int i = 2; i < N; i++)
	{
		ors[i - 1].Connect(ors[i - 2].Out(), wires[i]);
	}
}

template<unsigned int N>
inline void OrGateN<N>::Update()
{
	for (int i = 0; i < N - 1; i++)
	{
		ors[i].Update();
	}
}