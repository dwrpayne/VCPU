#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"

class Inverter : public Component
{
public:
	void Connect(const Wire& a);
	inline void Update()
	{
		out.Set(!in->On());
	}

	const Wire& Out() const { return out; }
	
private:
	const Wire* in;
	Wire out;
};


template <unsigned int N>
class InverterN : public Component
{
public:
	InverterN();
	void Connect(const Bundle<N>& wires);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<Inverter, N> invs;
	Bundle<N> out;
};

template<unsigned int N>
inline InverterN<N>::InverterN()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, invs[i].Out());
	}
}

template<unsigned int N>
inline void InverterN<N>::Connect(const Bundle<N>& wires)
{
	for (int i = 0; i < N; ++i)
	{
		invs[i].Connect(wires[i]);
	}
}

template<unsigned int N>
inline void InverterN<N>::Update()
{
	for (auto& inv : invs)
	{
		inv.Update();
	}
}
