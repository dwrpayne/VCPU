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
	void Connect(const Wire& clear, const Wire& enable);
	void Update();

	const Bundle<N>& Out() { return out; }


private:
	std::array<JKFlipFlop, N> bits;
	std::array<AndGate, N - 2> ands;
	std::array<AndGate, N> enables;
	Inverter clearInv;
	std::array<OrGate, N> kInput;
	std::array<AndGate, N> jInput;
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
inline void Counter<N>::Connect(const Wire& clear, const Wire& enable)
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
	clearInv.Connect(clear);
	for (int i = 0; i < N; ++i)
	{
		enables[i].Connect(inputs[i], enable);
		jInput[i].Connect(enables[i].Out(), clearInv.Out());
		kInput[i].Connect(enables[i].Out(), clear);
		bits[i].Connect(jInput[i].Out(), kInput[i].Out());
	}
}

template<unsigned int N>
inline void Counter<N>::Update()
{
	for (auto& a : ands)
	{
		a.Update();
	}
	clearInv.Update();
	for (int i = N - 1; i >= 0; --i)
	{
		enables[i].Update();
		jInput[i].Update();
		kInput[i].Update();
		bits[i].Update();
	}
}
