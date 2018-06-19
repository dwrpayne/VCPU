#pragma once
#include <array>
#include "Component.h"
#include "Bundle.h"
#include "Multiplexer.h"

// Masker applies the mask to the "maskee", and then stomps the mask bits of base with maskee
// Maskee 10101010101010101010101010101010
// Mask   00000011111111000000111111110000
// Base   11111111111111110000000000000000
// Result 11111110101010110000101010100000

template <unsigned int N>
class Masker : public Component
{
public:
	typedef Bundle<N> Word;
	Masker();
	void Connect(const Word& maskee, const Word& base, const Word& mask);
	void Update();
	const Word& Out() const { return out; }
	
private:
	std::array<Multiplexer<2>, N> muxes;
	Word out;
};

template<unsigned int N>
inline Masker<N>::Masker()
{
	for (int i = 0; i < N; i++)
	{
		out.Connect(i, muxes[i].Out());
	}
}

template<unsigned int N>
inline void Masker<N>::Connect(const Word& maskee, const Word& base, const Word& mask)
{
	for (int i = 0; i < N; i++)
	{
		muxes[i].Connect({ &base[i], &maskee[i] }, mask[i]);
	}
}

template<unsigned int N>
inline void Masker<N>::Update()
{
	for (int i = 0; i < N; i++)
	{
		muxes[i].Update();
	}
}
