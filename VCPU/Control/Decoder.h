#pragma once

#include "Component.h"
#include "AndGate.h"
#include "Inverter.h"
#include "Bundle.h"

template <unsigned int N>
class Decoder : public Component
{
public:
	static const unsigned int BITS = bits<N>;
	static const unsigned int N1 = pow(2, BITS / 2);		//N1 and N2 and the widths of the two sub-decoders
	static const unsigned int N2 = N / N1;

	Decoder();
	void Connect(const Bundle<BITS>& in);
	void Update();

	const Bundle<N>& Out() { return out; }

private:
	Decoder<N1> dec1;
	Decoder<N2> dec2;
};

template<unsigned int N>
inline Decoder<N>::Decoder()
{
}

template<unsigned int N>
inline void Decoder<N>::Connect(const Bundle<BITS>& in)
{
}

template<unsigned int N>
inline void Decoder<N>::Update()
{
}
