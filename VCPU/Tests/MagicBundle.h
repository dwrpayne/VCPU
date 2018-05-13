#pragma once
#include <assert.h>
#include <vector>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"

/***************
A MagicBundle is an outside-the-system tool that maintains arbitrary state on a collection of wires.
Used for testing only since it's "magic" with respect to the VCPU.
It can't be connected to input wires from the VCPU.
****************/

template <unsigned int N>
class MagicBundle : public Bundle<N>
{
public:
	MagicBundle() 
	{
		Init();
	}

	void Init()
	{
		for (int i = 0; i < N; i++)
		{
			wires[i] = &magicwires[i];
		}
	}

	explicit MagicBundle(int n)
	{
		assert((unsigned int)(abs(n) + (int)(n >= 0)) <= pow2(N-1));
		Init();
		Write(n);
	}

	explicit MagicBundle(unsigned int n)
	{
		assert(n < pow2(N));
		Init();
		Write(n);
	}

	void Write(int n)
	{
		value = n;

		bool negative = n < 0;
		n = abs(n) - (int)negative;

		assert(n <= pow(2, N));
		for (int i = 0; i < N - 1; i++)
		{
			magicwires[i].Set(negative ^ (bool)(n % 2));
			n /= 2;
		}
		magicwires[N-1].Set(negative);
	}

	void Write(unsigned int n)
	{
		int val = n < pow2(N) / 2 ? n : n - pow2(N);
		Write(val);
	}

private:
	std::array<Wire, N> magicwires;
	int value;
};