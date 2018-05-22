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

Cache::Cache()
{
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		writeMemCacheLine.Connect(i * WORD_SIZE, writeLineMuxes[i].Out());
	}
}

void Cache::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const CacheLineDataBundle& mem_data)
{
	auto byteAddr = addr.Range<bits(WORD_BYTES)>(0);
	auto wordAddr = addr.Range<ADDR_BITS - bits(WORD_BYTES)>(bits(WORD_BYTES));

	CacheOffsetBundle offset = wordAddr.Range<CACHE_OFFSET_BITS>(0);
	CacheIndexBundle index = wordAddr.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
	TagBundle tag = wordAddr.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);

	indexDecoder.Connect(index);
	writeEnable.Connect(indexDecoder.Out(), Bundle<NUM_CACHE_LINES>(cacheMiss.Out()));
	writeEnableOr.Connect(writeEnable.Out(), Bundle<NUM_CACHE_LINES>(write));

	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;

	cacheLineDataWriteMux.Connect({ mem_data, writeMemCacheLine }, cacheHitMux.Out());
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, offset, writeEnable.Out()[i], data, writeEnableOr.Out()[i], mem_data);
		cacheDataOuts[i] = cachelines[i].OutLine();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheMiss.Connect(cacheHitMux.Out());
	outCacheLineMux.Connect(cacheDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
	
	offsetDecoder.Connect(offset);
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		writeLineMuxes[i].Connect({ dataWordBundles[i], data }, offsetDecoder.Out()[i]);
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}
	writeToMem.Connect(cacheHitMux.Out(), write);
}

void Cache::Update()
{
	indexDecoder.Update();
	writeEnableOr.Update();
	writeEnable.Update();
	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitMux.Update();
	cacheMiss.Update();
	outCacheLineMux.Update();
	outDataMux.Update();

	offsetDecoder.Update();
	for (auto& mux : writeLineMuxes)
	{
		mux.Update();
	}
	writeToMem.Update();
}
