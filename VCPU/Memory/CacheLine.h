#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Matcher.h"

template <unsigned int N, unsigned int Nwords, unsigned int NTag>
class CacheLine : public Component
{
public:
	static const int LINE_BITS = N * Nwords;
	static const int OFFSET_BITS = bits(Nwords);
	typedef Bundle<N> WordBundle;
	typedef Bundle<OFFSET_BITS> OffsetBundle;
	typedef Bundle<LINE_BITS> LineBundle;

	CacheLine();
	void Connect(Bundle<NTag> tagin, const OffsetBundle& offset, const WordBundle& writewordmask, const WordBundle& dataword, const Wire& writeline, const LineBundle& dataline);
	void Update();

	const LineBundle& OutLine() { return outLineBundle; }
	const Wire& CacheHit() { return tagMatcher.Out(); }

private:
	Register<NTag> tag;
	Matcher<NTag> tagMatcher;
	Decoder<Nwords> offsetDecoder;
	MultiGate<OrGate, Nwords> wordwriteOr;
	MultiGate<OrGate, N> writeBitmask;
	std::array<RegisterMasked<N>, Nwords> words;
	std::array<MuxBundle<N, 2>, Nwords> writeDataMux;

	LineBundle outLineBundle;
#ifdef DEBUG
	template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES> friend class Cache;
#endif
};

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
inline CacheLine<N, Nwords, NTag>::CacheLine()
{
	for (int i = 0; i < Nwords; i++)
	{
		outLineBundle.Connect(i*N, words[i].Out());
	}
}

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
void CacheLine<N, Nwords, NTag>::Connect(Bundle<NTag> tagin, const OffsetBundle& offset, const WordBundle& writewordmask, const WordBundle& dataword, const Wire& writeline, const LineBundle& dataline)
{
	tag.Connect(tagin, writeline);
	tagMatcher.Connect(tag.Out(), tagin);
	offsetDecoder.Connect(offset, Wire::ON);
	wordwriteOr.Connect(offsetDecoder.Out(), Bundle<Nwords>(writeline));
	writeBitmask.Connect(writewordmask, Bundle<N>(writeline));
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Connect({ dataword, dataline.Range<N>(i*N) }, writeline);
		words[i].Connect(writeDataMux[i].Out(), writeBitmask.Out(), wordwriteOr.Out()[i]);
	}	
}

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
inline void CacheLine<N, Nwords, NTag>::Update()
{
	tag.Update();
	tagMatcher.Update();
	offsetDecoder.Update();
	wordwriteOr.Update();
	writeBitmask.Update();
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Update();
		words[i].Update();
	}
}
