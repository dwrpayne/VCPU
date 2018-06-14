#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Inverter.h"
#include "FullAdder.h"

template <unsigned int N>
class Comparator : public Component
{
public:
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update(); 

	const Wire& Equal() { return zeroOut.Out(); }
	const Wire& NonEqual() { return nonzeroOut.Out(); }
	const Wire& Less() { return adders.Out()[N-1]; }
	const Wire& LessEqual() { return lessEq.Out(); }
	const Wire& Greater() { return greater.Out(); }
	const Wire& GreaterEqual() { return greaterEq.Out(); }

private:
#if DEBUG
	Bundle<N> inA;
	Bundle<N> inB;
#endif
	InverterN<N> bInv;
	FullAdderN<N> adders;
	OrGateN<N> nonzeroOut;
	Inverter zeroOut;
	Inverter greaterEq;
	OrGate lessEq;
	AndGate greater;
};

template<unsigned int N>
inline void Comparator<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
#if DEBUG
	inA.Connect(0, a);
	inB.Connect(0, b);
#endif
	bInv.Connect(b);
	adders.Connect(a, bInv.Out(), Wire::ON);
	nonzeroOut.Connect(adders.Out());
	zeroOut.Connect(nonzeroOut.Out());
	lessEq.Connect(adders.Out()[N - 1], zeroOut.Out());
	greaterEq.Connect(adders.Out()[N - 1]);
	greater.Connect(greaterEq.Out(), nonzeroOut.Out());
}

template<unsigned int N>
inline void Comparator<N>::Update()
{
	bInv.Update();
	adders.Update();
	nonzeroOut.Update();
	zeroOut.Update();
	lessEq.Update();
	greaterEq.Update();
	greater.Update();
}
