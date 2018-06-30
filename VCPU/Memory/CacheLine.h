#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Matcher.h"
#include "JKFlipFlop.h"

template <unsigned int N, unsigned int NTag>
class CacheLine : public Component
{
public:
	typedef Bundle<N> LineBundle;
	typedef Bundle<NTag> TagBundle;

	void Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& writeline, const Wire& writeword);
	void Update();

	const TagBundle& Tag() { return tag.Out(); }
	const LineBundle& OutLine() { return lineRegister.Out(); }
	const Wire& CacheHit() { return tagMatchAndValid.Out(); }
	const Wire& Dirty() { return dirtyFlag.Q(); }
	const Wire& Valid() { return valid.Q(); }

private:
	Register<NTag> tag;
	Matcher<NTag> tagMatcher;
	JKFlipFlop valid;
	AndGate tagMatchAndValid;
	AndGate canWriteWord;
	OrGate writeEnable;
	DFlipFlop dirtyFlag;
	RegisterMasked<N> lineRegister;
#ifdef DEBUG
	template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS> friend class Cache;
#endif
};

template<unsigned int N, unsigned int NTag>
void CacheLine<N, NTag>::Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& writeline, const Wire& writeword)
{
	tag.Connect(tagin, writeline);
	tagMatcher.Connect(tag.Out(), tagin);
	valid.Connect(writeline, Wire::OFF);
	tagMatchAndValid.Connect(tagMatcher.Out(), valid.Q());

	canWriteWord.Connect(tagMatchAndValid.Out(), writeword);
	writeEnable.Connect(writeline, canWriteWord.Out());
	dirtyFlag.Connect(writeword, writeEnable.Out());
	lineRegister.Connect(dataline, linemask, writeEnable.Out());

}

template<unsigned int N, unsigned int NTag>
inline void CacheLine<N, NTag>::Update()
{
	tag.Update();
	tagMatcher.Update();
	valid.Update();
	tagMatchAndValid.Update();
	canWriteWord.Update();
	writeEnable.Update();
	dirtyFlag.Update();
	lineRegister.Update();
}
