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
	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite);
	void Update();

	const DataBundle& Out() const { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitMux.Out(); }
	const Wire& NeedStall() { return readMiss.Out(); }
	const Wire& NoStall() { return readMissInv.Out(); }
	
private:
	void UpdateCache();
	void UpdateMemory();

	Memory<WORD_SIZE, MAIN_MEMORY_BYTES, CACHE_LINE_BITS> mMemory;
	std::array<CacheLine<WORD_SIZE, CACHE_WORDS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Decoder<NUM_CACHE_LINES> indexDecoder;	
	MultiGate<AndGate, NUM_CACHE_LINES> writeEnable;
	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Inverter cacheMiss;
	AndGate readMiss;
	Inverter readMissInv;
	OrGate needWaitForMemory;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outDataMux;

	std::condition_variable mCondVar;
	std::mutex mMutex;
	bool mCacheWaitingOnMemory;
	std::thread memUpdateThread;
	
	friend class Debugger;
};


template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Cache()
	: mCacheWaitingOnMemory(false)
	, memUpdateThread(&Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateMemory, this)
{
	memUpdateThread.detach();
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Connect(const AddrBundle& addr, const DataBundle& data, 
																					const Wire& write, const Wire& read,
																					const Wire& bytewrite, const Wire& halfwrite)
{
	mMemory.Connect(addr, data, write, bytewrite, halfwrite);

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
		cachelines[i].Connect(tag, offset, write, data, writeEnable.Out()[i], mMemory.OutLine());
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheMiss.Connect(cacheHitMux.Out());
	readMiss.Connect(read, cacheMiss.Out());
	readMissInv.Connect(readMiss.Out());
	needWaitForMemory.Connect(readMiss.Out(), write);

	outCacheLineMux.Connect(cacheLineDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Update()
{
	UpdateCache();

	static const int MEM_UPDATE_FREQUENCY = 8;
	int mem_update_counter = 0;
	
	if (!(mem_update_counter % MEM_UPDATE_FREQUENCY))
	{
		mem_update_counter = 0;
		{
			std::lock_guard<std::mutex> lk(mMutex);
			mCacheWaitingOnMemory = true;
		}
		mCondVar.notify_one();
		{
			std::unique_lock<std::mutex> lk(mMutex);
			mCondVar.wait(lk, [this] {return !mCacheWaitingOnMemory; });
		}
	}
	mem_update_counter++;
}

template<unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateCache()
{
	std::unique_lock<std::mutex> lk(mMutex);

	indexDecoder.Update();
	writeEnable.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitMux.Update();
	cacheMiss.Update();
	readMiss.Update();
	readMissInv.Update();
	needWaitForMemory.Update();
	outCacheLineMux.Update();
	outDataMux.Update();
}

template <unsigned int WORD_SIZE, unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<WORD_SIZE, CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateMemory()
{
	while (true)
	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCondVar.wait(lk, [this] {return mCacheWaitingOnMemory; });

		mMemory.Update();
		mCacheWaitingOnMemory = false;
		lk.unlock();
		mCondVar.notify_one();

	}
}