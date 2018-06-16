#pragma once

#include "Component.h"
#include "DFlipFlop.h"
#include "DFlipFlopReset.h"
#include "Bundle.h"

template <unsigned int N>
class Register : public Component
{
public:
	static const int N = N;
	Register();
	void Connect(const Bundle<N>& data, const Wire& load);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<DFlipFlop, N> bits;
	Bundle<N> out;
};

template<unsigned int N>
inline Register<N>::Register()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}

template<unsigned int N>
inline void Register<N>::Update()
{
	for (int i = 0; i < N; ++i)
	{
		bits[i].Update();
	}
}

template<unsigned int N>
inline void Register<N>::Connect(const Bundle<N>& data, const Wire & load)
{
	for (int i = 0; i < N; ++i)
	{
		bits[i].Connect(data[i], load);
	}
}


template <unsigned int N>
class RegisterReset : public Component
{
public:
	RegisterReset();
	void Connect(const Bundle<N>& data, const Wire& load, const Wire& reset);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	Inverter resetInv;
	AndGate doLoad;
	std::array<DFlipFlopReset, N> bits;
	Bundle<N> out;
};

template<unsigned int N>
inline RegisterReset<N>::RegisterReset()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}

template<unsigned int N>
inline void RegisterReset<N>::Update()
{
	resetInv.Update();
	doLoad.Update();
	for (int i = 0; i < N; ++i)
	{
		bits[i].Update();
	}
}

template<unsigned int N>
inline void RegisterReset<N>::Connect(const Bundle<N>& data, const Wire & load, const Wire& reset)
{
	resetInv.Connect(reset);
	doLoad.Connect(load, resetInv.Out());
	for (int i = 0; i < N; ++i)
	{
		bits[i].Connect(data[i], doLoad.Out(), reset);
	}
}
