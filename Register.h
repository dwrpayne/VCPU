#pragma once

#include "Component.h"
#include "DFlipFlop.h"
#include "Bundle.h"

template <unsigned int N>
class Register : public Component
{
public:
	Register() 
	{
		for (int i = 0; i < N; ++i)
		{
			out.Connect(i, bits[i].Q());
		}
	}

	template<unsigned int N>
	void Connect(const Bundle<N>& data, const Wire& load)
	{
		for (int i = 0; i < N; ++i)
		{
			bits[i].Connect(data[i], load);
		}
	}

	void Update()
	{
		for (int i = 0; i < N; ++i)
		{
			bits[i].Update();
		}
	}

	const Bundle<N>& Out() { return out; }

private:
	DFlipFlop bits[N];
	Bundle<N> out;
};
