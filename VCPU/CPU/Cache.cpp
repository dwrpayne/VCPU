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

void Cache::Connect(const AddrBundle & addr, const CacheLineDataBundle & data, const Wire& write)
{
	CacheOffsetBundle offset = addr.Range<CACHE_OFFSET_BITS>(0);
	CacheIndexBundle index = addr.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
	TagBundle tag = addr.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);

	addrDecoder.Connect(index);
	writeEnable.Connect(addrDecoder.Out(), Bundle<NUM_CACHE_LINES>(cacheMiss.Out()));

	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, data, writeEnable.Out()[i]);
		cacheDataOuts[i] = cachelines[i].Out();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitAnd.Connect(cacheHitCollector);
	cacheMiss.Connect(cacheHitAnd.Out());
	outCacheLineMux.Connect(cacheDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
}

void Cache::Update()
{
	addrDecoder.Update();
	writeEnable.Update();
	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitAnd.Update();
	cacheMiss.Update();
	outCacheLineMux.Update();
	outDataMux.Update();
}
