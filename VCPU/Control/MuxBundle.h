#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Multiplexer.h"

template <unsigned int N, unsigned int Ninput>
class MuxBundle : public Component
{
public:
	static const unsigned int BITS = bits(Ninput);
	MuxBundle();
	void Connect(const std::array<Bundle<N>, Ninput> in, const Bundle<BITS>& sel);

	template <typename = typename std::enable_if<BITS==1, const>::type>
	void Connect(const std::array<Bundle<N>, Ninput> in, const Wire& sel)
	{
		Connect(in, Bundle<BITS>(sel));
	}

	void Update();

	const Bundle<N>& Out() { return out; }

private:
	Multiplexer<Ninput> muxes[N];
	Bundle<N> out;
};

template <unsigned int N, unsigned int Ninput>
inline MuxBundle<N, Ninput>::MuxBundle()
{
	for (int i = 0; i < N; i++)
	{
		out.Connect(i, muxes[i].Out());
	}
}

template <unsigned int N, unsigned int Ninput>
inline void MuxBundle<N, Ninput>::Connect(const std::array<Bundle<N>, Ninput> in, const Bundle<BITS>& sel)
{
	for (int i = 0; i < N; i++)
	{
		Bundle<Ninput> muxin;
		for (int input = 0; input < Ninput; input++)
		{
			muxin.Connect(input, in[input][i]);
		}
		muxes[i].Connect(muxin, sel);
	}
}

template <unsigned int N, unsigned int Ninput>
inline void MuxBundle<N, Ninput>::Update()
{
	for (int i = 0; i < N; i++)
	{
		muxes[i].Update();
	}
}
