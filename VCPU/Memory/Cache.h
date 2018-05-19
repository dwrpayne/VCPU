#pragma once
#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"

template <unsigned int N, unsigned int NReg>
class Cache : public Component
{
public:
	static const unsigned int WORD_LEN = N / 8;
	static const unsigned int BYTES = N * NReg * WORD_LEN;
	static const unsigned int ADDR_BITS = bits(NReg) + bits(WORD_LEN);
	static const unsigned int TAG_BITS = N - ADDR_BITS;
	typedef Bundle<TAG_BITS> TagBundle;
	typedef Bundle<N> DataBundle;

	void Connect(const Bundle<N>& addr, const DataBundle& data, const Wire& write);
	void Update();

	const DataBundle& Out() { return outMux.Out(); }
	const Wire& CacheHit() 

	virtual int Cost() const
	{
		return 0;
	}

private:
	Decoder<NReg> addrDecoder;
	MultiGate<AndGate, NReg> writeEnable;
	std::array<CacheLine<N, TAG_BITS>, NReg> cachelines;
	MuxBundle<N, NReg> outMux;
	AndGateN<NReg> cacheHit;

	friend class Debugger;
};

template<unsigned int N, unsigned int NReg>
inline void Cache<N, NReg>::Connect(const Bundle<N>& addr, const DataBundle & data, const Wire& write)
{
	auto byteAddr = addr.Range<0, bits(WORD_LEN)>();
	auto wordAddr = addr.Range<bits(WORD_LEN), ADDR_BITS>();
	TagBundle tagAddr = addr.Range<ADDR_BITS, N>();

	addrDecoder.Connect(wordAddr);
	writeEnable.Connect(addrDecoder.Out(), Bundle<NReg>(write));

	std::array<DataBundle, NReg> regOuts;
	Bundle<NReg> cacheHitBundle;
	for (int i = 0; i < NReg; ++i)
	{
		cachelines[i].Connect(tagAddr, data, )
		regOuts[i] = cachelines[i].Out();
		cacheHitBundle.Connect(i, cachelines[i].CacheHit());
	}
	outMux.Connect(regOuts, wordAddr);
	cacheHit.Connect(cacheHitBundle);
}

template<unsigned int N, unsigned int NReg>
inline void Cache<N, NReg>::Update()
{
	addrDecoder.Update();
	writeEnable.Update();
	for (auto& reg : registers)
	{
		reg.Update();
	}
	outMux.Update();
}
