#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Multiplexer.h"

template <unsigned int WIDTH, unsigned int N>
class MuxBundle : public Component
{
public:
	static const unsigned int BITS = bits(N);
	MuxBundle();
	void Connect(const std::array<Bundle<WIDTH>, N> in, const Bundle<BITS>& sel);
	void Update();

	const Bundle<WIDTH>& Out() { return out; }

private:
	Multiplexer<N> muxes[WIDTH];
	Bundle<WIDTH> out;
};

template <unsigned int WIDTH, unsigned int N>
inline MuxBundle<WIDTH, N>::MuxBundle()
{
	for (int i = 0; i < WIDTH; i++)
	{
		out.Connect(i, muxes[i].Out());
	}
}

template <unsigned int WIDTH, unsigned int N>
inline void MuxBundle<WIDTH, N>::Connect(const std::array<Bundle<WIDTH>, N> in, const Bundle<BITS>& sel)
{
	for (int i = 0; i < WIDTH; i++)
	{
		Bundle<N> muxin;
		for (int input = 0; input < N; input++)
		{
			muxin.Connect(input, in[input][i]);
		}
		muxes[i].Connect(muxin, sel);
	}
}

template <unsigned int WIDTH, unsigned int N>
inline void MuxBundle<WIDTH, N>::Update()
{
	for (int i = 0; i < WIDTH; i++)
	{
		muxes[i].Update();
	}
}
