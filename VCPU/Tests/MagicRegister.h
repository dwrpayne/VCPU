#pragma once
#include <vector>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"

/***************
A MagicRegister is an outside-the-system tool that maintains arbitrary state on a collection of wires.
It supports integer read/writes, Used for testing only since it's "magic" with respect to the VCPU.
It's a separate class so it doesn't pollute the Register class with magic integer write methods.
It can't be connected to input wires from the VCPU.
****************/

#if DEBUG

template <unsigned int N>
class MagicRegister
{
public:
	MagicRegister() 
	{
		for (int i = 0; i < N; i++)
		{
			out.Connect(i, wires[i]);
		}
	}

	const Wire& Get(unsigned int n) const
	{
		return wires[n];
	}

	const Wire& operator[](unsigned int n) const
	{
		return Get(n);
	}

	const Bundle<N>& Out() { return out; }

	const int width = N;

	void Write(unsigned int n)
	{
		value = n;
		for (int i = N - 1; i >= 0; i--)
		{
			wires[i].Set(n % 2);
			n /= 2;
		}
	}

private:
	std::array<Wire, N> wires;
	unsigned int value;
	Bundle<N> out;
};
#endif
