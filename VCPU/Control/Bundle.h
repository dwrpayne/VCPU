#pragma once
#include <vector>
#include "Wire.h"

/***************
A Bundle is a way to collect some wires and hand them to another component.
It doesn't really exist and can't be modified. 
If you reconnect its wires, you have to reconnect the components that are using it.
****************/

template <unsigned int N>
class Bundle
{
public:
	Bundle() {}
	Bundle(std::initializer_list<const Wire*> list)
	{
		Connect(list);
	}

	// Splits a single wire into an n-way Bundle.
	explicit Bundle(const Wire& wire)
	{
		for (int i = 0; i < N; ++i)
		{
			wires[i] = &wire;
		}
	}
	
	void Connect(std::initializer_list<const Wire*> list)
	{
		int i = 0;
		for (const auto& wire : list)
		{
			wires[i++] = wire;
		}
	}
	
	template <unsigned int M>
	void Connect(int start, const Bundle<M>& wires)
	{
		for (int i = 0; i < M; i++)
		{
			Connect(start + i, wires[i]);
		}
	}

	void Connect(int n, const Wire& wire)
	{
		wires[n] = &wire;
	}

	template <unsigned int start, unsigned int end>
	const Bundle<end - start> Range() const
	{
		Bundle<end - start> out;
		for (int i = start; i < end; i++)
		{
			out.Connect(i - start, Get(i));
		}
		return out;
	}

	const Wire& Get(unsigned int n) const 
	{
		return *wires[n];
	}

	const Wire& operator[](unsigned int n) const 
	{
		return Get(n);
	}

	template <unsigned int U=N, typename = typename std::enable_if<N==1, const >::type>
	operator const Wire&() const
	{
		return Get(0);
	}


	const int width = N;

	int Read() const
	{
		int n = 0;
		bool negative = Get(N-1).On();
		for (int i = N-2; i >= 0; --i)
		{
			n *= 2;
			n += (negative ^ Get(i).On()) ? 1 : 0;
		}
		return (negative ? -1 : 1) * n - (int)negative;
	}

	unsigned int UnsignedRead() const
	{
		int val = Read();
		return val >= 0 ? val : pow2(width) + val;
	}

protected:
	std::array<const Wire*, N> wires;
};

