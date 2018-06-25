#pragma once

#include "Component.h"
#include "DFlipFlop.h"
#include "DFlipFlopReset.h"
#include "Bundle.h"
#include "MultiGate.h"

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
class RegisterEnable : public Component
{
public:
	static const int N = N;
	void Connect(const Bundle<N>& data, const Wire& load, const Wire& enable);
	void Update();

	const Bundle<N>& Out() const { return out.Out(); }

private:
	Register<N> reg;
	MultiGate<AndGate, N> out;
};

template<unsigned int N>
inline void RegisterEnable<N>::Connect(const Bundle<N>& data, const Wire & load, const Wire& enable)
{
	reg.Connect(data, load);
	out.Connect(reg.Out(), Bundle<N>(enable));
}

template<unsigned int N>
inline void RegisterEnable<N>::Update()
{
	reg.Update();
	out.Update();
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

template <unsigned int N>
class RegisterMasked : public Component
{
public:
	RegisterMasked();
	void Connect(const Bundle<N>& data, const Bundle<N>& mask, const Wire& enable);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<DFlipFlop, N> bits;
	MultiGate<AndGate, N> writemask;
	Bundle<N> out;
};

template<unsigned int N>
inline RegisterMasked<N>::RegisterMasked()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}
template<unsigned int N>
inline void RegisterMasked<N>::Connect(const Bundle<N>& data, const Bundle<N>& mask, const Wire& enable)
{
	writemask.Connect(mask, Bundle<N>(enable));
	for (int i = 0; i < N; ++i)
	{
		bits[i].Connect(data[i], writemask.Out()[i]);
	}
}


template<unsigned int N>
inline void RegisterMasked<N>::Update()
{
	writemask.Update();
	for (int i = 0; i < N; ++i)
	{
		bits[i].Update();
	}
}
