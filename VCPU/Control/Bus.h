#pragma once
#include <array>
#include <algorithm> 

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Junction.h"


template <unsigned int N>
class Bus : public Bundle<N>
{
public:
	Bus()
	{
		for (int i = 0; i < N; i++)
		{
			wires[i] = &junctions[i];
		}
	}

	template <unsigned int M>
	void Connect(const Bundle<M>& bundle, int start = 0)
	{
		for (int i = 0; i < M; i++)
		{
			Connect(start + i, bundle[i]);
		}
	}

	template <unsigned int M>
	void Remove(const Bundle<M>& bundle, int start = 0)
	{
		for (int i = 0; i < M; i++)
		{
			Disconnect(start + i, bundle[i]);
		}
	}

	void Connect(int n, const Wire& wire)
	{
		junctions[n].Connect(wire);
	}

	void Disconnect(int n, const Wire& wire)
	{
		junctions[n].Disconnect(wire);
	}
private:
	std::array<Junction, N> junctions;
};

