#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "Memory.h"

class Cache : public Component
{
public:
	static const int MAIN_MEMORY_BYTES = 8192;
	static const int CACHE_SIZE_BYTES = 512;
	static const int WORD_SIZE = 32;
	static const int WORD_BYTES = WORD_SIZE / 8;
	static const int CACHE_LINE_BITS = 256;
	static const int ADDR_BITS = bits(MAIN_MEMORY_BYTES);
	static const int MAIN_MEMORY_LINES = MAIN_MEMORY_BYTES * 8 / CACHE_LINE_BITS;
	static const int NUM_CACHE_LINES = CACHE_SIZE_BYTES * 8 / CACHE_LINE_BITS;
	static const int CACHE_INDEX_BITS = bits(NUM_CACHE_LINES);
	static const int CACHE_WORDS = CACHE_LINE_BITS / WORD_SIZE;
	static const int CACHE_OFFSET_BITS = bits(CACHE_WORDS);
	static const int TAG_BITS = ADDR_BITS - CACHE_INDEX_BITS - CACHE_OFFSET_BITS - bits(WORD_BYTES);

	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<WORD_SIZE> DataBundle;
	typedef Bundle<CACHE_LINE_BITS> CacheLineDataBundle;
	typedef Bundle<CACHE_OFFSET_BITS> CacheOffsetBundle;
	typedef Bundle<CACHE_INDEX_BITS> CacheIndexBundle;
	typedef Bundle<TAG_BITS> TagBundle;

	void Connect(const AddrBundle& addr, const CacheLineDataBundle& data, const Wire& write);
	void Update();

	const DataBundle& Out() { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitAnd.Out(); }

private:
	std::array<CacheLine<CACHE_LINE_BITS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Decoder<NUM_CACHE_LINES> addrDecoder;
	MultiGate<AndGate, NUM_CACHE_LINES> writeEnable;
	AndGateN<NUM_CACHE_LINES> cacheHitAnd;
	Inverter cacheMiss;


	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outDataMux;

	friend class Debugger;
};
