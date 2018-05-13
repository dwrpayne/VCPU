#pragma once

#include "Component.h"
#include "AndGate.h"
#include "MultiGate.h"
#include "Inverter.h"
#include "Bundle.h"

template <unsigned int N>
class Decoder : public Component
{
public:
	static const unsigned int BITS = bits(N);
	static const unsigned int BITS1 = BITS / 2;
	static const unsigned int BITS2 = BITS - BITS1;
	static const unsigned int N1 = pow2(BITS1);		//N1 and N2 and the widths of the two sub-decoders
	static const unsigned int N2 = pow2(BITS2);

	void Connect(const Bundle<BITS>& in);
	void Update();

	const Bundle<N>& Out() { return ands.Out(); }

private:
	Decoder<N1> dec1;
	Decoder<N2> dec2;
	MultiGate<AndGate, N> ands;
};

template<unsigned int N>
inline void Decoder<N>::Connect(const Bundle<BITS>& in)
{
	dec1.Connect(in.Range<0, BITS1>());
	dec2.Connect(in.Range<BITS1, BITS>());

	Bundle<N> a, b;
	for (int i = 0; i < N; i+=N1)
	{
		a.Connect(i, dec1.Out());
		for (int j = 0; j < N1; j++)
		{
			b.Connect(i + j, dec2.Out()[i / N1]);
		}
	}
	for (int i = 0; i < N; ++i)
	{
		ands.Connect(a, b);
	}
}

template<unsigned int N>
inline void Decoder<N>::Update()
{
	dec1.Update();
	dec2.Update();
	ands.Update();
}

template <>
class Decoder<2> : public Component
{
public:
	Decoder();
	void Connect(const Bundle<1>& in);
	void Update();

	const Bundle<2>& Out() { return out; }

private:
	Inverter inv;
	Inverter buffer;
	Bundle<2> out;
};

inline Decoder<2>::Decoder()
{
	out.Connect({ &inv.Out(), &buffer.Out() });
}

inline void Decoder<2>::Connect(const Bundle<1>& in)
{
	inv.Connect(in);
	buffer.Connect(inv.Out());
}

inline void Decoder<2>::Update()
{
	inv.Update();
	buffer.Update();
}
