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

	void Write(long long n)
	{
		value = n;
		Bundle<N> b(n);
		for (int i = 0; i < N; ++i)
		{
			magicwires[i].Set(b[i].On());
		}
	}

	void Write(int n)
	{
		Write((long long)n);
	}

	void Write(unsigned int n)
	{
		int val = n < pow2(N) / 2 ? n : n - pow2(N);
		Write(val);
	}

private:
	std::array<Wire, N> magicwires;
	long long value;
};