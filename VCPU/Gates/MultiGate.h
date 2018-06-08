#pragma once

#include "Bundle.h"
#include "Component.h"

template <typename Gate, unsigned int N>
class MultiGate : public Component
{
public:
	MultiGate();
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<Gate, N> gates;
	Bundle<N> out;
};

template<typename Gate, unsigned int N>
inline MultiGate<Gate, N>::MultiGate()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, gates[i].Out());
	}
}

template<typename Gate, unsigned int N>
inline void MultiGate<Gate, N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	for (int i = 0; i < N; ++i)
	{
		gates[i].Connect(a[i], b[i]);
	}
}

template<typename Gate, unsigned int N>
inline void MultiGate<Gate, N>::Update()
{
	for (auto& gate : gates)
	{
		gate.Update();
	}
}


template <typename Gate, unsigned int Ninput, unsigned int Nway>
class MultiGateN : public Component
{
public:
	MultiGateN();
	void Connect(std::array<Bundle<Nway>, Ninput> in);
	void Update();

	const Bundle<Ninput>& Out() const { return out; }

private:
	std::array<Gate, Ninput> gates;
	Bundle<Ninput> out;
};

template <typename Gate, unsigned int Ninput, unsigned int Nway>
inline MultiGateN<Gate, Ninput, Nway>::MultiGateN()
{
	for (int i = 0; i < Ninput; ++i)
	{
		out.Connect(i, gates[i].Out());
	}
}

template <typename Gate, unsigned int Ninput, unsigned int Nway>
inline void MultiGateN<Gate, Ninput, Nway>::Connect(std::array<Bundle<Nway>, Ninput> in)
{
	for (int i = 0; i < Ninput; ++i)
	{
		gates[i].Connect(in[i]);
	}
}

template <typename Gate, unsigned int Ninput, unsigned int Nway>
inline void MultiGateN<Gate, Ninput, Nway>::Update()
{
	for (auto& gate : gates)
	{
		gate.Update();
	}
}


template <typename Gate, unsigned int N, unsigned int Ninput>
class MultiGateNBitwise : public Component
{
public:
	MultiGateNBitwise();
	void Connect(std::array<Bundle<N>, Ninput> in);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<Gate, N> gates;
	Bundle<N> out;
};

template <typename Gate, unsigned int N, unsigned int Ninput>
inline MultiGateNBitwise<Gate, N, Ninput>::MultiGateNBitwise()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, gates[i].Out());
	}
}

template <typename Gate, unsigned int N, unsigned int Ninput>
inline void MultiGateNBitwise<Gate, N, Ninput>::Connect(std::array<Bundle<N>, Ninput> in)
{
	for (int i = 0; i < N; ++i)
	{
		Bundle<Ninput> gatein;
		for (int bit = 0; bit < Ninput; bit++)
		{
			gatein.Connect(bit, in[bit][i]);
		}
		gates[i].Connect(gatein);
	}
}

template <typename Gate, unsigned int N, unsigned int Ninput>
inline void MultiGateNBitwise<Gate, N, Ninput>::Update()
{
	for (auto& gate : gates)
	{
		gate.Update();
	}
}
