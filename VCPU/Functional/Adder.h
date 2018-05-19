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
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<3>& sel);
	void Update();

	const Bundle<N>& Out() { return adders.Out(); }
	const Wire& Cout() { return adders.Cout(); }
	const Wire& Overflow() { return overflow.Out(); }

	virtual int Cost() const
	{
		return bInv.Cost() + bInputMux.Cost() + adders.Cost() + overflow.Cost();
	}

private:
	InverterN<N> bInv;
	MuxBundle<N, 4> bInputMux;
	FullAdderN<N> adders;

	OverflowDetector overflow;
};

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
	adders.Connect(a, bInputMux.Out(), sel[0]);
	overflow.Connect(a[N - 1], bInputMux.Out()[N - 1], adders.Out()[N-1], sel[2]);
}

template<unsigned int N>
inline void Adder<N>::Update()
{
	bInv.Update();
	bInputMux.Update();
	adders.Update();
	overflow.Update();
}
