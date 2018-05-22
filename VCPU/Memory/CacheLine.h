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
	void Connect(Bundle<NTag> tagin, Bundle<OFFSET_ADDR> offset, const Wire& writeword, const WordBundle& dataword, const Wire& writeline, const LineBundle& dataline);
	void Update();

	const WordBundle& OutWord() { return outWordMux.Out(); }
	const LineBundle& OutLine() { return outLineBundle; }
	const Wire& CacheHit() { return cacheHit.Out(); }

private:
	OrGate writeOr;
	Register<NTag> tag;
	MultiGate<XNorGate, NTag> tagMatcher;
	AndGateN<NTag> tagAllMatch; 
	DFlipFlop valid;
	AndGate cacheHit;

	AndGate writeWordAndHit;
	OrGate writeAllow;
	std::array<Register<N>, Nwords> words;
	Decoder<Nwords> offsetDecoder;
	std::array<MuxBundle<N, 2>, Nwords> writeDataMux;
	MultiGate<OrGate, Nwords> writeWordSelectOr;
	MultiGate<AndGate, Nwords> writeWordEnableAnd;

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
void CacheLine<N, Nwords, NTag>::Connect(Bundle<NTag> tagin, Bundle<OFFSET_ADDR> offset, const Wire& writeword, const WordBundle& dataword, const Wire& writeline, const LineBundle& dataline)
{
	writeOr.Connect(writeword, writeline);
	tag.Connect(tagin, writeline);
	tagMatcher.Connect(tag.Out(), tagin);
	tagAllMatch.Connect(tagMatcher.Out());
	valid.Connect(writeline, writeline);
	cacheHit.Connect(tagAllMatch.Out(), valid.Q());

	writeWordAndHit.Connect(writeword, cacheHit.Out());
	writeAllow.Connect(writeWordAndHit.Out(), writeline);
	offsetDecoder.Connect(offset);
	writeWordSelectOr.Connect(offsetDecoder.Out(), Bundle<Nwords>(writeline));
	writeWordEnableAnd.Connect(writeWordSelectOr.Out(), Bundle<Nwords>(writeAllow.Out()));
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Connect({ dataword, dataline.Range<N>(i*N) }, writeline);
		words[i].Connect(writeDataMux[i].Out(), writeWordEnableAnd.Out()[i]);
	}	
}

template<unsigned int N, unsigned int Nwords, unsigned int NTag>
inline void CacheLine<N, Nwords, NTag>::Update()
{
	writeOr.Update();
	tag.Update();
	tagMatcher.Update();
	tagAllMatch.Update();
	valid.Update();
	cacheHit.Update();

	writeWordAndHit.Update();
	writeAllow.Update();
	offsetDecoder.Update();
	writeWordSelectOr.Update();
	writeWordEnableAnd.Update();
	for (int i = 0; i < Nwords; i++)
	{
		writeDataMux[i].Update();
		words[i].Update();
	}
}
