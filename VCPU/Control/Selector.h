#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "OrGate.h"
#include <type_traits>

template <unsigned int N, unsigned int Ninput>
class Selector : public Component
{
public:
	typedef Bundle<N> DataBundle;
	typedef typename std::conditional<Ninput == 2, MultiGate<OrGate, N>, 
							MultiGateN<OrGateN<Ninput>, N, Ninput> >::type MultiOrGate;

	void Connect(const std::array<DataBundle, Ninput> in, const Bundle<Ninput>& sel);

	void Update();

	const DataBundle& Out() const { return orOut.Out(); }

private:
	std::array<MultiGate<AndGate, N>, Ninput> ands;
	MultiOrGate orOut;
};

template <unsigned int N, unsigned int Ninput>
inline void Selector<N, Ninput>::Connect(const std::array<DataBundle, Ninput> in, const Bundle<Ninput>& sel)
{
	std::array<Bundle<Ninput>, N> bundles;

	for (int i = 0; i < Ninput; i++)
	{
		ands[i].Connect(in[i], DataBundle(sel[i]));

		for (int bit = 0; bit < N; bit++)
		{
			bundles[bit].Connect(i, ands[i].Out()[bit]);
		}
	}
	orOut.Connect(bundles);
}

template <unsigned int N, unsigned int Ninput>
inline void Selector<N, Ninput>::Update()
{
	for (auto& and : ands)
	{
		and.Update();
	}
	orOut.Update();
}
