#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Inverter.h"
#include "FullAdder.h"
#include "Multiplexer.h"
#include "MuxBundle.h"
#include "OverflowDetector.h"

/*******************************
	  Adder select table
	S2 S1 S0 	Output
	-----------------
	0  0  0		A
	0  0  1		A + 1
	0  1  0		A - 1
	0  1  1		A
	1  0  0		A - B - 1
	1  0  1		A - B
	1  1  0		A + B 
	1  1  1		A + B + 1

*******************************/

template <unsigned int N>
class Adder : public Component
{
public:
	Adder();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<3>& sel);
	void Update();

	const Bundle<N>& Out() { return sum; }
	const Wire& Cout() { return adders[N - 1].Cout(); }
	const Wire& Overflow() { return overflow.Out(); }

private:
	InverterN<N> bInv;
	MuxBundle<N, 4> bInputMux;
	std::array<FullAdder, N> adders;
	Bundle<N> sum;

	OverflowDetector overflow;
};

template<unsigned int N>
inline Adder<N>::Adder()
{
	for (int i = 0; i < N; ++i)
	{
		sum.Connect(i, adders[i].S());
	}
}

template<unsigned int N>
inline void Adder<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<3>& sel)
{
	// S0 is Cin of low adder. b input for S1S2 pairs:
	// 00 - 0
	// 10 - B
	// 01 - Binv
	// 11 - 1

	bInv.Connect(b);
	bInputMux.Connect({ Bundle<N>::OFF, Bundle<N>::ON, bInv.Out(), b }, sel.Range<1, 3>());
	for (int i = 0; i < N; ++i)
	{
		const Wire& cin = (i > 0 ? adders[i - 1].Cout() : sel[0]);
		adders[i].Connect(a[i], bInputMux.Out()[i], cin);
	}

	overflow.Connect(a[N - 1], bInputMux.Out()[N - 1], adders[N - 1].S(), sel[2]);
}

template<unsigned int N>
inline void Adder<N>::Update()
{
	bInv.Update();
	bInputMux.Update();
	for (auto& adder : adders)
	{
		adder.Update();
	}

	overflow.Update();
}
