#pragma once

#include "Component.h"
#include "AndGate.h"
#include "MultiGate.h"
#include "Inverter.h"
#include "Bundle.h"


template <unsigned int N>
class Encoder : public Component
{
public:
	static const unsigned int BITS = bits(N);

	void Connect(const Bundle<N>& in);
	void Update();

	const Bundle<BITS>& Out() const { return ors.Out(); }
	
private:
	MultiGateN<OrGateN<N/2>, BITS, N/2> ors;
};

template<unsigned int N>
inline void Encoder<N>::Connect(const Bundle<N>& in)
{
	std::array<Bundle<N/2>, BITS> orIns;
	for (int i = 0; i < BITS; i++)
	{
		int wire = 0;
		for (int bit = 0; bit < N; bit++)
		{
			if (bit & (1 << i))
			{
				orIns[i].Connect(wire++, in[bit]);
			}
		}
	}
	ors.Connect(orIns);
}

template<unsigned int N>
inline void Encoder<N>::Update()
{
	ors.Update();
}


template <>
class Encoder<4> : public Component
{
public:
	static const unsigned int N = 4;
	static const unsigned int BITS = bits(N);

	void Connect(const Bundle<4>& in);
	void Update();

	const Bundle<BITS>& Out() const { return ors.Out(); }

private:
	MultiGate<OrGate, BITS> ors;
};

inline void Encoder<4>::Connect(const Bundle<4>& in)
{
	std::array<Bundle<N / 2>, BITS> orIns;
	for (int i = 0; i < BITS; i++)
	{
		int wire = 0;
		for (int bit = 0; bit < N; bit++)
		{
			if (bit & (1 << i))
			{
				orIns[i].Connect(wire++, in[bit]);
			}
		}
	}
	ors.Connect(orIns[0], orIns[1]);
}

inline void Encoder<4>::Update()
{
	ors.Update();
}