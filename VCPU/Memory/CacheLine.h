#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "MultiGate.h"
#include "XNorGate.h"

template <unsigned int N, unsigned int NTag>
class CacheLine : public Component
{
public:
	void Connect(Bundle<NTag> tag, Bundle<N> datain, const Wire& write);
	void Update();

	const Bundle<N>& Out() { return data.Out(); }
	const Wire& CacheHit() { return cacheHit.Out(); }

	virtual int Cost() const
	{
		return data.Cost() + tag.Cost() + tagAnd.Cost() + tagMatch.Cost() + status.Cost() + cacheHit.Cost();
	}

private:
	Register<N> data;
	Register<NTag> tag;
	MultiGate<XNorGate, NTag> tagAnd;
	AndGateN<NTag> tagMatch; 
	DFlipFlop status;
	AndGate cacheHit;
};

template<unsigned int N, unsigned int NTag>
inline void CacheLine<N, NTag>::Connect(Bundle<NTag> tagin, Bundle<N> datain, const Wire& write)
{
	data.Connect(datain, write);
	tag.Connect(tagin, write);
	tagAnd.Connect(tag.Out(), tagin);
	tagMatch.Connect(tagAnd.Out());
	status.Connect(write, write);
	cacheHit.Connect(tagMatch.Out(), status.Q());
}

template<unsigned int N, unsigned int NTag>
inline void CacheLine<N, NTag>::Update()
{
	data.Update();
	tag.Update();
	tagAnd.Update();
	tagMatch.Update();
	status.Update();
	cacheHit.Update();
}
