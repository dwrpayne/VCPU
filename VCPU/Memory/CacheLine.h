#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Matcher.h"
#include "JKFlipFlop.h"

template <unsigned int N, unsigned int Nwords, unsigned int NTag>
class CacheLine : public Component
{
public:
	static const int LINE_BITS = N * Nwords;
	static const int OFFSET_BITS = bits(Nwords);
	typedef Bundle<N> WordBundle;
	typedef Bundle<OFFSET_BITS> OffsetBundle;
	typedef Bundle<LINE_BITS> LineBundle;
	typedef Bundle<NTag> TagBundle;

	CacheLine();
	void Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& enable, const Wire& dirty);
	void Update();

	const TagBundle& Tag() { return tag.Out(); }
	const LineBundle& OutLine() { return outLineBundle; }
	const Wire& CacheHit() { return tagMatchAndValid.Out(); }
	const Wire& Dirty() { return dirtyFlag.Q(); }
	const Wire& Valid() { return valid.Q(); }

private:
	AndGateN<N> writeLine;
	AndGate writeTag;
	Register<NTag> tag;
	Matcher<NTag> tagMatcher;
	JKFlipFlop valid;
	AndGate tagMatchAndValid;
	AndGate cacheHitEnabled;
	OrGate writing;
	AndGate updateDirtyFlag;
	DFlipFlop dirtyFlag;
	Decoder<Nwords> offsetDecoder;
	MultiGate<OrGate, Nwords> wordwriteOr;
	MultiGate<AndGate, Nwords> writeEnable;
	MultiGate<OrGate, N> writeBitmask;
	std::array<RegisterMasked<N>, Nwords> words;
	std::array<MuxBundle<N, 2>, Nwords> writeDataMux;

	LineBundle outLineBundle;
#ifdef DEBUG
	template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS> friend class Cache;
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
void CacheLine<N, Nwords, NTag>::Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& enable, const Wire& dirty)
{
	writeTag.Connect(writeline, enable);
	tag.Connect(tagin, writeTag.Out());
	tagMatcher.Connect(tag.Out(), tagin);
	valid.Connect(writeTag.Out(), Wire::OFF);
	tagMatchAndValid.Connect(tagMatcher.Out(), valid.Q());
	cacheHitEnabled.Connect(tagMatchAndValid.Out(), enable );
	writing.Connect(writeline, dirty);
	updateDirtyFlag.Connect(writing.Out(), cacheHitEnabled.Out());
	dirtyFlag.Connect(dirty, updateDirtyFlag.Out());
	offsetDecoder.Connect(wordoffset, Wire::ON);
	wordwriteOr.Connect(offsetDecoder.Out(), Bundle<Nwords>(writeline));
	writeEnable.Connect(wordwriteOr.Out(), Bundle<Nwords>(cacheHitEnabled.Out()));
	writeBitmask.Connect(writewordmask, Bundle<N>(writeline));
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Connect({ dataword, dataline.Range<N>(i*N) }, writeline);
		words[i].Connect(writeDataMux[i].Out(), writeBitmask.Out(), writeEnable.Out()[i]);
	}	
}

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
inline void CacheLine<N, Nwords, NTag>::Update()
{
	writeTag.Update();
	tag.Update();
	tagMatcher.Update();
	valid.Update();
	tagMatchAndValid.Update();
	cacheHitEnabled.Update();
	writing.Update();
	updateDirtyFlag.Update();
	dirtyFlag.Update();
	offsetDecoder.Update();
	wordwriteOr.Update();
	writeEnable.Update();
	writeBitmask.Update();
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Update();
		words[i].Update();
	}
}
