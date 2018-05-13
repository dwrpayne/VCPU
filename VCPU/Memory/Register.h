#pragma once

#include "Component.h"
#include "DFlipFlop.h"
#include "Bundle.h"

template <unsigned int N>
class Register : public Component
{
public:
	Register();
	void Connect(const Bundle<N>& data, const Wire& load);
	void Update();

	const Bundle<N>& Out() { return out; }

private:
	DFlipFlop BITS[N];
	Bundle<N> out;
};

template<unsigned int N>
inline Register<N>::Register()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, BITS[i].Q());
	}
}

template<unsigned int N>
inline void Register<N>::Update()
{
	for (int i = 0; i < N; ++i)
	{
		BITS[i].Update();
	}
}

template<unsigned int N>
inline void Register<N>::Connect(const Bundle<N>& data, const Wire & load)
{
	for (int i = 0; i < N; ++i)
	{
		BITS[i].Connect(data[i], load);
	}
}
