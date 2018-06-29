#pragma once
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sstream>

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "Masker.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "Memory.h"
#include "ByteMask.h"
#include "SystemBus.h"
#include "TriStateBuffer.h"
#include "BusRequestBuffer.h"

template <unsigned int N>
class CacheLineMasker : public Component
{
public:
	typedef Bundle<N> CacheLine;
	void Connect(const Bundle<2> byteindex, const Bundle<bits(N)-5> wordoffset, const Bundle<32>& dataword, const CacheLine& dataline,
		const Wire& bytewrite, const Wire& halfwrite, const Wire& wordwrite)
	{
		maskedDataWord.Connect(byteindex, dataword, bytewrite, halfwrite, wordwrite);
		lineDataShifter.Connect(maskedDataWord.Word().ZeroExtend<N>(), wordoffset.ShiftZeroExtend<bits(N)>(5));
		lineMaskShifter.Connect(maskedDataWord.WordMask().ZeroExtend<N>(), wordoffset.ShiftZeroExtend<bits(N)>(5));
		maskedCacheLine.Connect(lineDataShifter.Out(), dataline, lineMaskShifter.Out());
		lineFullMask.Connect({ CacheLine::ON, lineMaskShifter.Out() }, wordwrite);
	}
	void Update()
	{
		maskedDataWord.Update();
		lineDataShifter.Update();
		lineMaskShifter.Update();
		maskedCacheLine.Update();
		lineFullMask.Update();
	}
	const CacheLine& Line() const { return maskedCacheLine.Out(); }
	const CacheLine& LineMask() const { return lineFullMask.Out(); }

private:
	WordMasker maskedDataWord;
	LeftShifter<N> lineDataShifter;
	LeftShifter<N> lineMaskShifter;
	MuxBundle<N, 2> lineFullMask;
	Masker<N> maskedCacheLine;
};

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
class Cache : public Component
{
public:
	static const int WORD_SIZE = 32;
	static const int ADDR_BITS = 32;
	static const int CACHE_BYTES = CACHE_SIZE_BYTES;
	static const int CACHE_LINE_BYTES = CACHE_LINE_BITS / 8;


	static const int WORD_BYTES = WORD_SIZE / 8;
	static const int BYTE_INDEX_BITS = bits(WORD_BYTES);
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
	typedef RequestBuffer<CACHE_LINE_BITS, ADDR_BITS, 4, 4> ReqBuffer;

	~Cache();
	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite, SystemBus & bus);
	void Update();
	void UpdateUntilNoStall(bool flush = false);

	const DataBundle& Out() const { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitMux.Out(); }
	const Wire& NeedStall() { return needStall.Out(); }
	const Wire& NoStall() { return needStallInv.Out(); }

	
private:
	class CacheAddrBundle : public AddrBundle
	{
	public:
		CacheAddrBundle() {}
		CacheAddrBundle(const AddrBundle& other)
		{
			Connect(0, other);
			byteIndex = Range<BYTE_INDEX_BITS>(0);
			wordIndex = Range<ADDR_BITS - BYTE_INDEX_BITS>(BYTE_INDEX_BITS);
			wordOffsetInLine = wordIndex.Range<CACHE_OFFSET_BITS>(0);
			cacheLineIndex = wordIndex.Range<CACHE_INDEX_BITS>(CACHE_OFFSET_BITS);
			tag = wordIndex.Range<TAG_BITS>(CACHE_OFFSET_BITS + CACHE_INDEX_BITS);
		}
		Bundle<BYTE_INDEX_BITS> ByteIndex() { return byteIndex; }
		Bundle<ADDR_BITS - BYTE_INDEX_BITS> WordIndex() { return wordIndex; }
		CacheOffsetBundle WordOffsetInLine() { return wordOffsetInLine; }
		CacheIndexBundle CacheLineIndex() { return cacheLineIndex; }
		TagBundle Tag() { return tag; }
		void SetTag(const TagBundle& t) { tag.Connect(0, t); Connect(ADDR_BITS - TAG_BITS, t); }

	private:
		Bundle<BYTE_INDEX_BITS> byteIndex; 
		Bundle<ADDR_BITS - BYTE_INDEX_BITS> wordIndex;
		CacheOffsetBundle wordOffsetInLine;
		CacheIndexBundle cacheLineIndex;
		TagBundle tag;
	};

	void DisconnectFromBus();
	SystemBus * pSystemBus;

	//ReqBuffer buffer;

	AndGateN<16> unCacheableAddr; // Replace this with a lookup table of some kind, or a tag!
	Inverter cacheableAddr;
	AndGate uncachedWrite;
	AndGate uncachedRead;
	OrGate uncachedReadOrWrite;
	OrGate cachedReadOrWrite;
	Matcher<ADDR_BITS> addrReadMatcher;
	AndGate gotCacheableDataFromMemory;
	AndGate gotResponseFromMemory;
	Inverter gotResponseFromMemoryInv;
	AndGate waitingForUncachedData;
	AndGate cachelinewrite;

	std::array<CacheLine<CACHE_LINE_BITS, TAG_BITS>, NUM_CACHE_LINES> cachelines;
	Register<WORD_SIZE> uncachedBuffer;
	RegisterEnable<WORD_SIZE> uncachedWriteBuffer;

	Decoder<NUM_CACHE_LINES> indexDecoder;

	CacheLineMasker<CACHE_LINE_BITS> lineWriteMasker;
	
	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Multiplexer<NUM_CACHE_LINES> cacheDirtyMux;
	AndGate evictedDirty;
	Inverter notEvictedDirty;
	Inverter cacheMissInternal;
	OrGate readOrWrite;
	AndGate cacheMiss;
	AndGate cacheableAddrCacheMiss;
	AndGate writeBufferFull;
	OrGate needStall;
	Inverter needStallInv;
	AndGateN<3> cachedReadReqOkay;
	OrGate shouldSendReadReq;
	OrGate shouldSendWriteReq;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<CACHE_LINE_BITS, 2> outDataToBusMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outCacheDataMux;
	MuxBundle<WORD_SIZE, 2> outDataMux;

	MuxBundle<TAG_BITS, NUM_CACHE_LINES> lineTagMux;
	MuxBundle<ADDR_BITS, 2> memAddrMux;

	BusRequestBuffer<CACHE_LINE_BITS, ADDR_BITS, 8> busBuffer;

	int cycles;

#ifdef DEBUG
	CacheAddrBundle DEBUG_addr;
#endif

	friend class Debugger;
};

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
inline Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::~Cache()
{
	DisconnectFromBus();
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& read,
	const Wire& write, const Wire& bytewrite, const Wire& halfwrite, SystemBus & bus)
{
	CacheAddrBundle address(addr);
#ifdef DEBUG
	DEBUG_addr = address;
#endif

	pSystemBus = &bus;	
	readOrWrite.Connect(read, write);

	unCacheableAddr.Connect(addr.Range<16>(16));
	cacheableAddr.Connect(unCacheableAddr.Out());
	uncachedWrite.Connect(unCacheableAddr.Out(), write);
	uncachedRead.Connect(unCacheableAddr.Out(), read);
	uncachedReadOrWrite.Connect(uncachedRead.Out(), uncachedWrite.Out());
	waitingForUncachedData.Connect(busBuffer.WaitingForResponse(), uncachedReadOrWrite.Out());
			
	// If we got data from memory, mask it in with the data we want to write.
	// This produces either the buffer read line with "data", if write is true, and a 32-bit shifted mask
	// Or just the buffer read line, if write is false, and a mask of all ones.
	lineWriteMasker.Connect(address.ByteIndex(), address.WordOffsetInLine(), data, busBuffer.OutRead(), bytewrite, halfwrite, write);

	// Temp Bundles for the cache line array
	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	std::array<TagBundle, NUM_CACHE_LINES> cacheLineTagOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;
	Bundle<NUM_CACHE_LINES> cacheDirtyCollector;
	
	cachelinewrite.Connect(cacheableAddr.Out(), busBuffer.ReadSuccess());
	CacheIndexBundle index = address.CacheLineIndex();
	indexDecoder.Connect(index, cachelinewrite.Out());

	// Cache Lines
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(address.Tag(),  lineWriteMasker.LineMask(), lineWriteMasker.Line(), indexDecoder.Out()[i], write);
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheLineTagOuts[i] = cachelines[i].Tag();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
		cacheDirtyCollector.Connect(i, cachelines[i].Dirty());
	}
	uncachedBuffer.Connect(busBuffer.OutRead().Range<32>(), unCacheableAddr.Out());

	// Evicted tag collector and addr calculation
	lineTagMux.Connect(cacheLineTagOuts, index);
	CacheAddrBundle evictedCacheAddr(addr);
	evictedCacheAddr.SetTag(lineTagMux.Out());
	memAddrMux.Connect({ addr, evictedCacheAddr }, evictedDirty.Out());

	// Status Flags
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheDirtyMux.Connect(cacheDirtyCollector, index);
	cacheMissInternal.Connect(cacheHitMux.Out());
	cacheMiss.Connect(cacheMissInternal.Out(), readOrWrite.Out());
	cacheableAddrCacheMiss.Connect(cacheMiss.Out(), cacheableAddr.Out());

	// We need to know when we have finished writing back the evicted dirty line. If we had a response from memory by now, the write succeeded.
	// This logic will need to change when I have a write buffer.
	evictedDirty.Connect(cacheableAddrCacheMiss.Out(), cacheDirtyMux.Out());
	notEvictedDirty.Connect(evictedDirty.Out());
	needStall.Connect(busBuffer.WaitingForResponse(), waitingForUncachedData.Out());
	needStallInv.Connect(needStall.Out());

	shouldSendReadReq.Connect(cacheableAddrCacheMiss.Out(), uncachedRead.Out());
	shouldSendWriteReq.Connect(evictedDirty.Out(), uncachedWrite.Out());

	// Output 
	outCacheLineMux.Connect(cacheLineDataOuts, index);
	outDataToBusMux.Connect({ outCacheLineMux.Out(), data.ZeroExtend<CACHE_LINE_BITS>() }, uncachedWrite.Out());
	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}
	outCacheDataMux.Connect(dataWordBundles, address.WordOffsetInLine());
	outDataMux.Connect({ outCacheDataMux.Out(), uncachedBuffer.Out() }, unCacheableAddr.Out());

	busBuffer.Connect(bus, outDataToBusMux.Out(), memAddrMux.Out(), addr, shouldSendWriteReq.Out(), shouldSendReadReq.Out());
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::Update()
{
	readOrWrite.Update();
	unCacheableAddr.Update();
	cacheableAddr.Update();
	uncachedWrite.Update();
	uncachedRead.Update();
	uncachedReadOrWrite.Update();
	waitingForUncachedData.Update();

	lineWriteMasker.Update();
	cachelinewrite.Update();
	indexDecoder.Update();

	// Must update the line tag mux *before* the registers, as their tags get stomped on write
	lineTagMux.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
	uncachedBuffer.Update();
	cacheHitMux.Update();
	cacheDirtyMux.Update();
	cacheMissInternal.Update();
	cacheMiss.Update();
	cacheableAddrCacheMiss.Update();
	evictedDirty.Update();
	notEvictedDirty.Update();
	needStall.Update();
	needStallInv.Update();

	// We do write-before-read because writes aren't buffered to the bus yet.
	// This needs to change to a read first, write buffer, query the buffer first on cache miss, architecture.
	shouldSendWriteReq.Update();
	shouldSendReadReq.Update();

	outCacheLineMux.Update();
	outDataToBusMux.Update();
	outCacheDataMux.Update();
	outDataMux.Update();
	memAddrMux.Update();

	busBuffer.Update();
	
	cycles++;
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::UpdateUntilNoStall(bool flush)
{
	do
	{
		Update();
	} while (NeedStall().On());// || (flush && buffer.WritePending().On()));
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::DisconnectFromBus()
{
}