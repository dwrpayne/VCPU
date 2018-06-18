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

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES=2048>
class Cache : public Component
{
public:
	static const int WORD_SIZE = 32;
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
	typedef Memory<WORD_SIZE, CACHE_LINE_BITS, MAIN_MEMORY_BYTES> MemoryType;

	Cache();
	virtual ~Cache();
	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite);
	void Update();
	void UpdateUntilNoStall(bool flush = false);

	const DataBundle& Out() const { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitMux.Out(); }
	const Wire& NeedStall() { return needStall.Out(); }
	const Wire& NoStall() { return needStallInv.Out(); }
	
private:	
	typename MemoryType::ReqBuffer buffer;
	MemoryType mMemory;
	std::array<CacheLine<WORD_SIZE, CACHE_WORDS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Matcher<ADDR_BITS> addrReadMatcher;
	AndGateN<3> gotResultFromMemory;
	Decoder<NUM_CACHE_LINES> indexDecoder;	
	
	MuxBundle<WORD_SIZE, 4> bytemasker;
	MuxBundle<WORD_SIZE, 2> halfmasker;
	MuxBundle<WORD_SIZE, 4> masker;
	LeftShifter<WORD_SIZE> dataByteShifter;

	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Inverter cacheMiss;
	AndGate readMiss;
	AndGateN<3> writeBufferFull;
	OrGate needStall;
	Inverter needStallInv;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outDataMux;

	bool exit;
	std::condition_variable mCV;
	std::mutex mMutex;
	bool memoryReady;
	std::thread memUpdateThread;
	int cycles;

#ifdef DEBUG
	AddrBundle DEBUG_addr;
#endif
	
	friend class Debugger;
};


template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Cache()
	: exit(false)
	, memoryReady(false)
	, mMemory(mMutex, mCV, memoryReady, exit)
	, memUpdateThread(&MemoryType::ThreadedUpdate, &mMemory)
{
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::~Cache()
{
	exit = true;
	mCV.notify_all();
	memUpdateThread.join();
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& read,
															const Wire& write, const Wire& bytewrite, const Wire& halfwrite)
{
#ifdef DEBUG
	DEBUG_addr.Connect(0, addr);
#endif
	buffer.Connect(addr, data, write, read);
	mMemory.Connect(buffer);

	auto byteAddr = addr.Range<bits(WORD_BYTES)>(0);
	auto wordAddr = addr.Range<ADDR_BITS - bits(WORD_BYTES)>(bits(WORD_BYTES));

	CacheOffsetBundle offset = wordAddr.Range<CACHE_OFFSET_BITS>(0);
	CacheIndexBundle index = wordAddr.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
	TagBundle tag = wordAddr.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);

	addrReadMatcher.Connect(addr, mMemory.ReadAddr());
	gotResultFromMemory.Connect({ &addrReadMatcher.Out(), &cacheMiss.Out(), &mMemory.ServicedRead() });
	indexDecoder.Connect(index, gotResultFromMemory.Out());

	bytemasker.Connect({ Bundle<WORD_SIZE>(0xff), Bundle<WORD_SIZE>(0xff00), Bundle<WORD_SIZE>(0xff0000), Bundle<WORD_SIZE>(0xff0000) }, byteAddr);
	halfmasker.Connect({ Bundle<WORD_SIZE>(0xffff), Bundle<WORD_SIZE>(0xffff0000) }, byteAddr[1]);
	masker.Connect({ Bundle<WORD_SIZE>::ON, bytemasker.Out(), halfmasker.Out(), Bundle<WORD_SIZE>::ON }, { &bytewrite, &halfwrite });

	dataByteShifter.Connect(data, { &Wire::OFF, &Wire::OFF, &Wire::OFF, &byteAddr[0], &byteAddr[1] });
	
	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;

	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(tag, offset, masker.Out(), dataByteShifter.Out(), indexDecoder.Out()[i], mMemory.OutLine());
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
	}
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheMiss.Connect(cacheHitMux.Out());
	readMiss.Connect(read, cacheMiss.Out());
	writeBufferFull.Connect({ &cacheMiss.Out(), &write, &buffer.WriteFull() });
	needStall.Connect(readMiss.Out(), writeBufferFull.Out());
	needStallInv.Connect(readMiss.Out());

	outCacheLineMux.Connect(cacheLineDataOuts, index);

	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}

	outDataMux.Connect(dataWordBundles, offset);
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Update()
{
	addrReadMatcher.Update();
	gotResultFromMemory.Update();
	indexDecoder.Update();

	bytemasker.Update();
	halfmasker.Update();
	masker.Update();
	dataByteShifter.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
#ifdef DEBUG
	if (gotResultFromMemory.Out().On())
	{
		std::cout << "Cache got ";
		auto& line = cachelines[(DEBUG_addr.UnsignedRead() / 32) % cachelines.size()];
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


	if (!(cycles % mMemory.MEMORY_UPDATE_RATIO))
	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCV.wait(lk, [this] {return !memoryReady; });
		memoryReady = true;
		lk.unlock();
		mCV.notify_all();
	}
	cycles++;
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateUntilNoStall(bool flush)
{
	Update();
	while (NeedStall().On() || (flush && buffer.WritePending().On()))
	{
		Update();
	}
}
