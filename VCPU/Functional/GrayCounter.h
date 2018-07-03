#pragma once

#include "Component.h"
#include "Inverter.h"

template <unsigned int N>
class Bundle;

template <unsigned int N>
class GrayCounter : public Component
{
public:
	void Connect(const Wire& clear, const Wire& enable);
	void Update();

	const Bundle<N>& OutBin() const { return counter.Out(); }
	const Bundle<N>& OutGray() const { return gray.Out(); }

private:
	Counter<N> counter;
	BinaryToGray<N> gray;
};

template<unsigned int N>
inline void GrayCounter<N>::Connect(const Wire& clear, const Wire& enable)
{
	counter.Connect(clear, enable);
	gray.Connect(counter.Out());
}

template<unsigned int N>
inline void GrayCounter<N>::Update()
{
	counter.Update();
	gray.Update();
}

