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

#if DEBUG

template <unsigned int N>
class MagicBundle : public Bundle<N>
{
public:
	MagicBundle() 
	{
		for (int i = 0; i < N; i++)
		{
			wires[i] = &magicwires[i];
		}
	}

	explicit MagicBundle(int n)
	{
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

private:
	std::array<Wire, N> magicwires;
	int value;
};
#endif
