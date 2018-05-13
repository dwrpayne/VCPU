#pragma once

#include "Component.h"
#include "Bundle.h"
#include "JKFlipFlop.h"
#include "AndGate.h"

template <unsigned int N>
class Counter : public Component
{
public:
	Counter();
	void Update();

	const Bundle<N>& Out() { return out; }

private:
	std::array<JKFlipFlop, N> bits;
	std::array<AndGate, N - 2> ands;
	Bundle<N> out;
};

template<unsigned int N>
inline Counter<N>::Counter()
{
	bits[0].Connect(Wire::ON, Wire::ON);
	bits[1].Connect(bits[0].Q(), bits[0].Q());
	ands[0].Connect(bits[0].Q(), bits[1].Q());
	bits[2].Connect(ands[0].Out(), ands[0].Out());
	for (int i = 1; i < N - 2; ++i)
	{
		ands[i].Connect(ands[i-1].Out(), bits[i + 1].Q());
		bits[i + 2].Connect(ands[i].Out(), ands[i].Out());
	}

	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}

template<unsigned int N>
inline void Counter<N>::Update()
{
	for (auto& a : ands)
	{
		a.Update();
	}
	for (int i = N-1; i >= 0; --i)
	{
		bits[i].Update();
	}

}
