#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Inverter.h"
#include "FullAdder.h"


template <unsigned int N>
class Adder : public Component
{
public:
	Adder();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& mode);
	void Update();

	const Bundle<N>& S() { return sum; }
	const Wire& C() { return adders[N - 1].Cout(); }

private:
	XorGateW<N> xors;
	FullAdder adders[N];
	Bundle<N> sum;
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
inline void Adder<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& mode)
{
	xors.Connect(b, Bundle<N>(mode));
	for (int i = 0; i < N; ++i)
	{
		const Wire& cin = (i > 0 ? adders[i - 1].Cout() : mode);
		adders[i].Connect(a[i], xors.Out()[i], cin);
	}
}

template<unsigned int N>
inline void Adder<N>::Update()
{
	xors.Update();
	for (auto& adder : adders)
	{
		adder.Update();
	}
}
