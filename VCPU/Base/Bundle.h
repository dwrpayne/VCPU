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

	void Connect(int n, const Wire& wire)
	{
		wires[n] = &wire;
	}

	const Wire& Get(unsigned int n) const 
	{
		return *wires[n];
	}

	const Wire& operator[](unsigned int n) const 
	{
		return Get(n);
	}

	const int width = N;

#ifdef DEBUG
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
#endif

private:
	std::array<const Wire*, N> wires;
};
