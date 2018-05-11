#pragma once

#include "Wire.h"
#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "Component.h"

class XorGate : public Component
{
public:
	void Connect(const Wire& a, const Wire& b);
	void Update();

	const Wire& Out() { return and.Out(); }

private:
	NandGate nand;
	OrGate or;
	AndGate and;
};


template <unsigned int N>
class XorGateN : public Component
{
public:
	XorGateN();
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update();

	const Bundle<N>& Out() { return out; }

private:
	XorGate xors[N];
	Bundle<N> out;
};

template<unsigned int N>
inline XorGateN<N>::XorGateN()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, xors[i].Out());
	}
}

template<unsigned int N>
inline void XorGateN<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	for (int i = 0; i < N; ++i)
	{
		xors[i].Connect(a[i], b[i]);
	}
}

template<unsigned int N>
inline void XorGateN<N>::Update()
{
	for (auto& xor : xors)
	{
		xor.Update();
	}
}
