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
	static const unsigned int BITS = 2;

	void Connect(const Bundle<4>& in);
	void Update();

	const Bundle<2>& Out() const { return ors.Out(); }

private:
	MultiGate<OrGate, 2> ors;
};

inline void Encoder<4>::Connect(const Bundle<4>& in)
{
	ors.Connect({ &in[1], &in[2] }, { &in[3], &in[3] });
}

inline void Encoder<4>::Update()
{
	ors.Update();
}