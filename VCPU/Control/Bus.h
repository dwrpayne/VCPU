#pragma once
#include <array>
#include <algorithm> 

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

	void Connect(const BundleAny& bundle, int start = 0)
	{
		for (unsigned int i = 0; i < bundle.Size(); i++)
		{
			Connect(start + i, bundle.Get(i));
		}
	}

	void Remove(const BundleAny& bundle, int start = 0)
	{
		for (unsigned int i = 0; i < bundle.Size(); i++)
		{
			Disconnect(start + i, bundle.Get(i));
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

