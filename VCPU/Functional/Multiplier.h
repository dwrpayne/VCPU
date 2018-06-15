#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Inverter.h"
#include "FullAdder.h"
#include "Multiplexer.h"
#include "MuxBundle.h"
#include "OverflowDetector.h"

template <unsigned int N>
class Multiplier : public Component
{
public:
	Multiplier();
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update();

	const Bundle<N*2>& Out() const { return out; }

private:
	std::array<MultiGate<AndGate, N>, N> ands;
	std::array<CarrySaveAdderN<N>, N-1> adders;
	FullAdderN<N> outAdder;
	Bundle<N * 2> out;
};

template<unsigned int N>
inline Multiplier<N>::Multiplier()
{
	out.Connect(0, ands[0].Out()[0]);
	for (int i = 0; i < N-1; ++i)
	{
		out.Connect(i+1, adders[i].Out()[0]);
	}
	out.Connect(N, outAdder.Out());
}

template<unsigned int N>
inline void Multiplier<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	for (int i = 0; i < N; i++)
	{
		ands[i].Connect(Bundle<N>(a[i]), b);
	}
	adders[0].Connect(ands[1].Out(), ands[0].Out().ShiftRightWireExtend<N>(1, Wire::OFF), Bundle<N>::OFF);
	for (int i = 1; i < N-1; i++)
	{
		adders[i].Connect(ands[i + 1].Out(), adders[i - 1].Out().ShiftRightWireExtend<N>(1, Wire::OFF),
			adders[i - 1].Cout());
	}
	outAdder.Connect(adders[N - 2].Out().ShiftRightWireExtend<N>(1, Wire::OFF), adders[N - 2].Cout(), Wire::OFF);
}

template<unsigned int N>
inline void Multiplier<N>::Update()
{
	for (int i = 0; i < N; i++)
	{
		ands[i].Update();
	}
	for (int i = 0; i < N-1; i++)
	{
		adders[i].Update();
	}
	outAdder.Update();
}
