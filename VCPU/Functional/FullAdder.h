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

	const Bundle<N>& Out() const { return sum; }
	const Wire& Cout() { return adders[N - 1].Cout(); }

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


template <unsigned int N>
class CarrySaveAdderN : public Component
{
public:
	CarrySaveAdderN();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<N>& cin);
	void Update();

	const Bundle<N>& Out() const { return sum; }
	const Bundle<N>& Cout() { return carries; }

private:
#if DEBUG
	Bundle<N> in1, in2, inc;
#endif
	std::array<FullAdder, N> adders;
	Bundle<N> sum;
	Bundle<N> carries;
};

template<unsigned int N>
inline CarrySaveAdderN<N>::CarrySaveAdderN()
{
	for (int i = 0; i < N; ++i)
	{
		sum.Connect(i, adders[i].S());
		carries.Connect(i, adders[i].Cout());
	}
}

template<unsigned int N>
inline void CarrySaveAdderN<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<N>& cin)
{
#if DEBUG
	in1.Connect(0, a);
	in2.Connect(0, b);
	inc.Connect(0, cin);
#endif
	for (int i = 0; i < N; ++i)
	{
		adders[i].Connect(a[i], b[i], cin[i]);
	}
}

template<unsigned int N>
inline void CarrySaveAdderN<N>::Update()
{
	for (int i = 0; i < N; ++i)
	{
		adders[i].Update();
	}
}

