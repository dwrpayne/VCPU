#pragma once
#include <array>
#include "Component.h"
#include "MuxBundle.h"
#include "Bundle.h"
#include "Extender.h"


// Given N-bit word and a selector Bundle, mask out the selected partial word and output it in the least-sig part of out
// Defaults to a byte masker
template <unsigned int N, unsigned int Nsubsize=8>
class SubWordSelector : public Component
{
public:
	static const int PARTS = N / Nsubsize;
	static const int ADDR = bits(PARTS);
	void Connect(const Bundle<N>& in, const Bundle<ADDR>& sel, const Wire& signextend);
	void Update();

	const Bundle<N>& Out() const { return extender.Out(); }

private:
	MuxBundle<Nsubsize, PARTS> byteMux;
	Extender<Nsubsize, N> extender;
};

template <unsigned int N, unsigned int Nsubsize>
inline void SubWordSelector<N, Nsubsize>::Connect(const Bundle<N>& in, const Bundle<ADDR>& sel, const Wire & signextend)
{
	std::array<Bundle<Nsubsize>, PARTS> bytes;
	for (int i = 0; i < PARTS; i++)
	{
		bytes[i] = in.Range<Nsubsize>(i * Nsubsize);
	}
	byteMux.Connect(bytes, sel);
	extender.Connect(byteMux.Out(), signextend);
}

template <unsigned int N, unsigned int Nsubsize>
inline void SubWordSelector<N, Nsubsize>::Update()
{
	byteMux.Update();
	extender.Update();
}
