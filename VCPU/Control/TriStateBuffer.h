#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"


class TriState : public Component
{
public:
	void Connect(const Wire& in, const Wire& e);
	void Update();

	const Wire& Out() const { return out; }

private:
	const Wire* input;
	const Wire* enable;
	Wire out;
};

template <unsigned int N>
class TriStateN : public Component
{
public:
	TriStateN();
	void Connect(const Bundle<N>& in, const Wire& e);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<TriState, N> buffers;
	Bundle<N> out;
};

template<unsigned int N>
inline TriStateN<N>::TriStateN()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(buffers[i].Out());
	}
}

template<unsigned int N>
inline void TriStateN<N>::Connect(const Bundle<N>& in, const Wire & e)
{
	for (int i = 0; i < N; ++i)
	{
		buffers.Connect(in[i], e);
	}
}

template<unsigned int N>
inline void TriStateN<N>::Update()
{
	for (int i = 0; i < N; ++i)
	{
		buffers.Update();
	}
}
