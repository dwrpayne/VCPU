#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "MultiGate.h"
#include "Decoder.h"
#include "XNorGate.h"

template <unsigned int N, unsigned int Nwords, unsigned int NTag>
class CacheLine : public Component
{
public:
	static const int LINE_BITS = N * Nwords;
	static const int OFFSET_ADDR = bits(Nwords);
	typedef Bundle<N> WordBundle;
	typedef Bundle<LINE_BITS> LineBundle;

	CacheLine();
	void Connect(Bundle<NTag> tagin, Bundle<OFFSET_ADDR> offset, const Wire& writesrc, const WordBundle& dataword, const Wire& writeenable, const LineBundle& dataline);
	void Update();

	const WordBundle& OutWord() { return outWordMux.Out(); }
	const LineBundle& OutLine() { return outLineBundle; }
	const Wire& CacheHit() { return cacheHit.Out(); }

private:
	AndGate tagWriteAnd;
	Register<NTag> tag;
	MultiGate<XNorGate, NTag> tagMatcher;
	AndGateN<NTag> tagAllMatch; 
	DFlipFlop valid;
	AndGate cacheHit;

	AndGate writeAllow;
	std::array<Register<N>, Nwords> words;
	Decoder<Nwords> offsetDecoder;
	std::array<MuxBundle<N, 2>, Nwords> writeMux;
	MultiGate<OrGate, Nwords> writeWordSelect;
	MultiGate<AndGate, Nwords> writeWordEnable;

	LineBundle outLineBundle;
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
void CacheLine<N, Nwords, NTag>::Connect(Bundle<NTag> tagin, Bundle<OFFSET_ADDR> offset, const Wire& writesrc, const WordBundle& dataword, const Wire& writeenable, const LineBundle& dataline)
{
	tagWriteAnd.Connect(writesrc, writeenable);
	tag.Connect(tagin, tagWriteAnd.Out());
	tagMatcher.Connect(tag.Out(), tagin);
	tagAllMatch.Connect(tagMatcher.Out());
	valid.Connect(writesrc, writesrc);
	cacheHit.Connect(tagAllMatch.Out(), valid.Q());

	writeAllow.Connect(cacheHit.Out(), writeenable);
	offsetDecoder.Connect(offset);
	writeWordSelect.Connect(offsetDecoder.Out(), Bundle<Nwords>(writesrc));
	writeWordEnable.Connect(writeWordSelect.Out(), Bundle<Nwords>(writeAllow.Out()));
	std::array<WordBundle, Nwords> wordOutBundles;
	for (int i = 0; i < Nwords; i++)
	{
		writeMux[i].Connect({ dataword, dataline.Range<N>(i*N) }, writesrc);
		words[i].Connect(writeMux[i].Out(), writeWordEnable.Out()[i]);
		wordOutBundles[i] = words[i].Out();
	}	
}

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
inline void CacheLine<N, Nwords, NTag>::Update()
{
	tagWriteAnd.Update();
	tag.Update();
	tagMatcher.Update();
	tagAllMatch.Update();
	valid.Update();
	cacheHit.Update();
	writeAllow.Update();
	offsetDecoder.Update();

	for (int i = 0; i < Nwords; i++)
	{
		writeMux[i].Update();
	}
	writeWordSelect.Update();
	writeWordEnable.Update();
	for (int i = 0; i < Nwords; i++)
	{
		words[i].Update();
	}
}
