#pragma once
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "Memory.h"

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES=2048>
class Cache : public Component
{
public:
	static const int CACHE_BYTES = CACHE_SIZE_BYTES;
	static const int MEMORY_BYTES = MAIN_MEMORY_BYTES;

	static const int WORD_BYTES = WORD_SIZE / 8;
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

	Cache();
	virtual ~Cache();
	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite);
	void Update();
	void UpdateUntilNoStall();

	const DataBundle& Out() const { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitMux.Out(); }
	const Wire& NeedStall() { return needStall.Out(); }
	const Wire& NoStall() { return needStallInv.Out(); }
	
private:
	void UpdateCache();
	void UpdateMemory();
	
	RequestBuffer<WORD_SIZE, ADDR_BITS, 8> buffer;
	Memory<WORD_SIZE, MAIN_MEMORY_BYTES, CACHE_LINE_BITS> mMemory;
	std::array<CacheLine<WORD_SIZE, CACHE_WORDS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Decoder<NUM_CACHE_LINES> indexDecoder;	
	Matcher<ADDR_BITS> addrReadMatcher;
	AndGateN<3> gotResultFromMemory;
	MultiGate<AndGate, NUM_CACHE_LINES> writeEnable;
	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Inverter cacheMiss;
	AndGate readMiss;
	AndGateN<3> writeBufferFull;
	OrGate needStall;
	Inverter needStallInv;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outDataMux;

	std::condition_variable mCondVar;
	std::mutex mMutex;
	volatile int mCacheState;
	std::thread memUpdateThread;

#ifdef DEBUG
	AddrBundle DEBUG_addr;
#endif
	
	friend class Debugger;
};


template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Cache()
	: mCacheState(0)
	, memUpdateThread(&Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateMemory, this)
{
}

template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::~Cache()
{
	mCacheState = 2;
	memUpdateThread.join();
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Connect(const AddrBundle& addr, const DataBundle& data, 
																					const Wire& write, const Wire& read,
																					const Wire& bytewrite, const Wire& halfwrite)
{
#ifdef DEBUG
	DEBUG_addr.Connect(0, addr);
#endif
	buffer.Connect(addr, data, { &write, &bytewrite, &halfwrite });
	mMemory.Connect(buffer, read, addr);

	auto byteAddr = addr.Range<bits(WORD_BYTES)>(0);
	auto wordAddr = addr.Range<ADDR_BITS - bits(WORD_BYTES)>(bits(WORD_BYTES));

	CacheOffsetBundle offset = wordAddr.Range<CACHE_OFFSET_BITS>(0);
	CacheIndexBundle index = wordAddr.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
	TagBundle tag = wordAddr.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);

	indexDecoder.Connect(index);
	addrReadMatcher.Connect(addr, mMemory.ReadAddr());
	gotResultFromMemory.Connect({ &addrReadMatcher.Out(), &readMiss.Out(), &mMemory.ServicedRead() });
	writeEnable.Connect(indexDecoder.Out(), Bundle<NUM_CACHE_LINES>(gotResultFromMemory.Out()));
	
	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;

	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, offset, write, data, writeEnable.Out()[i], mMemory.OutLine());
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheMiss.Connect(cacheHitMux.Out());
	readMiss.Connect(read, cacheMiss.Out());
	writeBufferFull.Connect({ &cacheMiss.Out(), &write, &buffer.Full() });
	needStall.Connect(readMiss.Out(), writeBufferFull.Out());
	needStallInv.Connect(readMiss.Out());

	outCacheLineMux.Connect(cacheLineDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
	mCacheState = 1;
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Update()
{
	UpdateCache();
}

template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateUntilNoStall()
{
	UpdateCache();
	while (NeedStall().On())
	{
		UpdateCache();
	}
}

template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateCache()
{
	indexDecoder.Update();
	addrReadMatcher.Update();
	gotResultFromMemory.Update();
	writeEnable.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
#ifdef DEBUG
	if (gotResultFromMemory.Out().On())
	{
		std::cout << "Cache got ";
		auto& line = cachelines[(DEBUG_addr.UnsignedRead()/8) % cachelines.size()];
		for (auto& reg : line.words)
		{
			std::cout << reg.Out().Read() << ", ";
		}
		std::cout << " on " << DEBUG_addr.UnsignedRead() << std::endl;
	}
#endif
	cacheHitMux.Update();
	cacheMiss.Update();
	readMiss.Update();
	writeBufferFull.Update();
	needStall.Update();
	needStallInv.Update();
	outCacheLineMux.Update();
	outDataMux.Update();
	buffer.Update();
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateMemory()
{
	while (mCacheState < 2)
	{
		if (mCacheState == 1)
		{
			mMemory.Update();
		}
	}
}