#pragma once
#include <vector>
#include "Wire.h"

// A bundle of wires

template <unsigned int N>
class Bundle 
{
public:
	Bundle()
	{
		for (int i = 0; i < N; i++)
		{
			wires.push_back(&WIRE_DISCONNECTED);
		}
	}
	Bundle(std::initializer_list<const Wire*> list)
	{
		Connect(list);
	}

	void Connect(std::initializer_list<const Wire*> list)
	{
		wires.clear();
		for (const auto& wire : list)
		{
			wires.push_back(wire);
		}
	}

	const Wire& Get(unsigned int n)
	{
		return *wires[n];
	}

	const Wire& operator[](unsigned int n)
	{
		return Get(n);
	}

	const int width = N;

private:
	std::vector<const Wire*> wires;
};

