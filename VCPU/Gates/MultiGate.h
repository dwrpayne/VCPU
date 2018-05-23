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
