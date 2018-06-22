#pragma once

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"

template <unsigned int N>
class BinaryToGray : public Component
{
public:
	BinaryToGray();
	void Connect(const Bundle<N> binary);
	void Update();

	const Bundle<N>& Out() const { return out; }
	
private:
	AndGate msb;
	std::array<XorGate, N - 1> xors;
	Bundle<N> out;
};

template<unsigned int N>
inline BinaryToGray<N>::BinaryToGray()
{
	out.Connect(N-1, msb.Out());
	for (int i = 0; i < N-1; ++i)
	{
		out.Connect(i, xors[i].Out());
	}
}

template<unsigned int N>
inline void BinaryToGray<N>::Connect(const Bundle<N> binary)
{
	msb.Connect(Wire::ON, binary[N - 1]);
	for (int i = 0; i < N - 1; ++i)
	{
		xors[i].Connect(binary[i], binary[i + 1]);
	}
}

template<unsigned int N>
inline void BinaryToGray<N>::Update()
{
	msb.Update();
	for (auto& xor : xors)
	{
		xor.Update();
	}
}


template <unsigned int N>
class GrayToBinary : public Component
{
public:
	GrayToBinary();
	void Connect(const Bundle<N> binary);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	AndGate msb;
	std::array<XorGate, N - 1> xors;
	Bundle<N> out;
};

template<unsigned int N>
inline GrayToBinary<N>::GrayToBinary()
{
	out.Connect(N-1, msb.Out());
	for (int i = 0; i < N - 1; ++i)
	{
		out.Connect(i, xors[i].Out());
	}
}

template<unsigned int N>
inline void GrayToBinary<N>::Connect(const Bundle<N> binary)
{
	msb.Connect(Wire::ON, binary[N - 1]);
	for (int i = 0; i < N - 2; ++i)
	{
		xors[i].Connect(binary[i], xors[i+1].Out());
	}
	xors[N - 2].Connect(binary[N - 2], binary[N - 1]);
}

template<unsigned int N>
inline void GrayToBinary<N>::Update()
{
	msb.Update();
	for (int i = N - 1; i > 0; --i)
	{
		xors[i-1].Update();
	}
}