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


void Cache::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const CacheLineDataBundle& mem_data)
{
	read.Connect(write);

	auto byteAddr = addr.Range<bits(WORD_BYTES)>(0);
	auto wordAddr = addr.Range<ADDR_BITS - bits(WORD_BYTES)>(bits(WORD_BYTES));

	CacheOffsetBundle offset = wordAddr.Range<CACHE_OFFSET_BITS>(0);
	CacheIndexBundle index = wordAddr.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
	TagBundle tag = wordAddr.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);

	indexDecoder.Connect(index);
	writeEnable.Connect(indexDecoder.Out(), Bundle<NUM_CACHE_LINES>(readMiss.Out()));

	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;

	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, offset, write, data, writeEnable.Out()[i], mem_data);
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheMiss.Connect(cacheHitMux.Out());
	readMiss.Connect(read.Out(), cacheMiss.Out());

	outCacheLineMux.Connect(cacheLineDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
}

void Cache::Update()
{
	indexDecoder.Update();
	writeEnable.Update();
	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitMux.Update();
	cacheMiss.Update();
	read.Update();
	readMiss.Update();
	outCacheLineMux.Update();
	outDataMux.Update();
}
