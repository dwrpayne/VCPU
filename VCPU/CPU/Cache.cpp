#pragma once
#include "Cache.h"
#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "Memory.h"

inline void Cache::Connect(const AddrBundle & addr, const DataBundle & data, const Wire& write)
{
	CacheOffsetBundle offset = addr.Range<0,CACHE_OFFSET_BITS>();
	CacheIndexBundle index = addr.Range<CACHE_OFFSET_BITS, CACHE_OFFSET_BITS + CACHE_INDEX_BITS>();
	TagBundle tag = addr.Range<CACHE_OFFSET_BITS + CACHE_INDEX_BITS, ADDR_BITS>();

	addrDecoder.Connect(index);
	writeEnable.Connect(addrDecoder.Out(), Bundle<NUM_CACHE_LINES>(write));

	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, { &Wire::OFF }, writeEnable.Out()[i]);
		cacheDataOuts[i] = cachelines[i].Out();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitAnd.Connect(cacheHitCollector);
	outCacheLineMux.Connect(cacheDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_LINE_BITS; i+=WORD_SIZE)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i);
	}

	outDataMux.Connect(dataWordBundles, offset);
}

inline void Cache::Update()
{
	addrDecoder.Update();
	writeEnable.Update();
	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitAnd.Update();
	outCacheLineMux.Update();
	outDataMux.Update();
}
