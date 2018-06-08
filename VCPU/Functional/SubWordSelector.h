#pragma once
#include <array>
#include "Component.h"
#include "MuxBundle.h"
#include "Bundle.h"


// Given N-bit word and a selector Bundle, mask out the selected partial word and output it in the least-sig part of out
// Defaults to a byte masker
template <unsigned int N, unsigned int Nsubsize=8>
class SubWordSelector : public Component
{
public:
	static const int PARTS = N / Nsubsize;
	static const int ADDR = bits(PARTS);
	SubWordSelector();
	void Connect(const Bundle<N>& in, const Bundle<ADDR>& sel, const Wire& signextend);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	MuxBundle<Nsubsize, PARTS> byteMux;
	Multiplexer<2> fillMux;
	Bundle<N> out;
};

template <unsigned int N, unsigned int Nsubsize>
inline SubWordSelector<N, Nsubsize>::SubWordSelector()
	: out(fillMux.Out())
{
	out.Connect(0, byteMux.Out());
}

template <unsigned int N, unsigned int Nsubsize>
inline void SubWordSelector<N, Nsubsize>::Connect(const Bundle<N>& in, const Bundle<ADDR>& sel, const Wire & signextend)
{
	std::array<Bundle<Nsubsize>, PARTS> bytes;
	for (int i = 0; i < PARTS; i++)
	{
		bytes[i] = in.Range<Nsubsize>(i * Nsubsize);
	}
	byteMux.Connect(bytes, sel);
	fillMux.Connect({ &byteMux.Out()[Nsubsize-1], &Wire::OFF }, signextend);
}

template <unsigned int N, unsigned int Nsubsize>
inline void SubWordSelector<N, Nsubsize>::Update()
{
	byteMux.Update();
	fillMux.Update();
}
