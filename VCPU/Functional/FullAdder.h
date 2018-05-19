#pragma once

#include "Component.h"
#include "Wire.h"
#include "OrGate.h"
#include "AndGate.h"
#include "XorGate.h"


class FullAdder : public Component
{
public:
	void Connect(const Wire& a, const Wire& b, const Wire& c);
	void Update();

	const Wire& S() { return xorOut.Out(); }
	const Wire& Cout() { return orGate.Out(); }

	virtual int Cost() const
	{
		return xorIn.Cost() + andIn.Cost() + xorOut.Cost() + andOut.Cost() + orGate.Cost();
	} 

private:
	XorGate xorIn;
	AndGate andIn;
	XorGate xorOut;
	AndGate andOut;
	OrGate orGate;
};

template <unsigned int N>
class FullAdderN : public Component
{
public:
	FullAdderN();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& cin);
	void Update();

	const Bundle<N>& Out() { return sum; }
	const Wire& Cout() { return adders[N - 1].Cout(); }

	virtual int Cost() const
	{
		return ArrayCost(adders);
	}

private:
	std::array<FullAdder, N> adders;
	Bundle<N> sum;
};

template<unsigned int N>
inline FullAdderN<N>::FullAdderN()
{
	for (int i = 0; i < N; ++i)
	{
		sum.Connect(i, adders[i].S());
	}
}

template<unsigned int N>
inline void FullAdderN<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& cin)
{
	for (int i = 0; i < N; ++i)
	{
		const Wire& in = (i > 0 ? adders[i - 1].Cout() : cin);
		adders[i].Connect(a[i], b[i], in);
	}
}

template<unsigned int N>
inline void FullAdderN<N>::Update()
{
	for (int i = 0; i < N; ++i)
	{
		adders[i].Update();
	}
}