#pragma once

#include "Component.h"
#include "Bundle.h"
#include "JKFlipFlop.h"
#include "AndGate.h"
#include "MuxBundle.h"
#include "Inverter.h"

template <unsigned int N>
class Counter : public Component
{
public:
	Counter();
	void Connect(const Bundle<N>& data, const Wire& load);
	void Update();

	const Bundle<N>& Out() { return out; }

private:
	std::array<JKFlipFlop, N> bits;
	std::array<AndGate, N - 2> ands;
	MuxBundle<N, 2> loadMux;
	InverterN<N> invData;
	Bundle<N> out;
};

template<unsigned int N>
inline Counter<N>::Counter()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}

template<unsigned int N>
inline void Counter<N>::Connect(const Bundle<N>& data, const Wire & load)
{
	Bundle<N> inputs;
	inputs.Connect(0, Wire::ON);
	inputs.Connect(1, bits[0].Q());
	inputs.Connect(2, ands[0].Out());

	ands[0].Connect(bits[0].Q(), bits[1].Q());
	for (int i = 1; i < N - 2; ++i)
	{
		ands[i].Connect(ands[i - 1].Out(), bits[i + 1].Q());
		inputs.Connect(i+2, ands[i].Out());
	}
	for (int i = 0; i < N; ++i)
	{
		bits[i].Connect(inputs[i], inputs[i]);
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
