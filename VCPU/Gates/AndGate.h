#pragma once
#include <array>

#include "Wire.h"
#include "Bundle.h"
#include "Component.h"


class AndGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return out; }

private:
	const Wire* in1;
	const Wire* in2;
	Wire out;
};


template <unsigned int N>
class AndGateN : public Component
{
public:
	void Connect(const Bundle<N>& wires);
	void Update();

	const Wire& Out() { return ands[N-2].Out(); }

private:
	std::array<AndGate, N-1> ands;
};


//////////////////
// AndGate N-way
//////////////////
template<unsigned int N>
inline void AndGateN<N>::Connect(const Bundle<N>& wires)
{
	ands[0].Connect(wires[0], wires[1]);
	for (int i = 2; i < N; i++)
	{
		ands[i - 1].Connect(ands[i - 2].Out(), wires[i]);
	}
}

template<unsigned int N>
inline void AndGateN<N>::Update()
{
	for (int i = 0; i < N - 1; i++)
	{
		ands[i].Update();
	}
}