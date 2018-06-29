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

	void Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& enable, const Wire& dirty);
	void Update();

	const TagBundle& Tag() { return tag.Out(); }
	const LineBundle& OutLine() { return lineRegister.Out(); }
	const Wire& CacheHit() { return tagMatchAndValid.Out(); }
	const Wire& Dirty() { return dirtyFlag.Q(); }
	const Wire& Valid() { return valid.Q(); }

private:
	Inverter notDirty;
	AndGate writeLine;
	AndGate writeTag;
	Register<NTag> tag;
	Matcher<NTag> tagMatcher;
	JKFlipFlop valid;
	AndGate tagMatchAndValid;
	AndGate cacheHitEnabled;
	OrGate writing;
	AndGate updateDirtyFlag;
	DFlipFlop dirtyFlag;
	RegisterMasked<N> lineRegister;
#ifdef DEBUG
	template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS> friend class Cache;
#endif
};

template<unsigned int N, unsigned int NTag>
void CacheLine<N, NTag>::Connect(const TagBundle& tagin, const LineBundle& linemask, const LineBundle& dataline, const Wire& enable, const Wire& dirty)
{
	notDirty.Connect(dirty);
	writeLine.Connect(enable, notDirty.Out());
	writeTag.Connect(writeLine.Out(), enable);
	tag.Connect(tagin, writeTag.Out());
	tagMatcher.Connect(tag.Out(), tagin);
	valid.Connect(writeTag.Out(), Wire::OFF);
	tagMatchAndValid.Connect(tagMatcher.Out(), valid.Q());
	cacheHitEnabled.Connect(tagMatchAndValid.Out(), enable );
	writing.Connect(writeLine.Out(), dirty);
	updateDirtyFlag.Connect(writing.Out(), cacheHitEnabled.Out());
	dirtyFlag.Connect(dirty, updateDirtyFlag.Out());

	lineRegister.Connect(dataline, linemask, writing.Out());

}

template<unsigned int N, unsigned int NTag>
inline void CacheLine<N, NTag>::Update()
{
	notDirty.Update();
	writeLine.Update();
	writeTag.Update();
	tag.Update();
	tagMatcher.Update();
	valid.Update();
	tagMatchAndValid.Update();
	cacheHitEnabled.Update();
	writing.Update();
	updateDirtyFlag.Update();
	dirtyFlag.Update();
	lineRegister.Update();
}
