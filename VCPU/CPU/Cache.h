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
#include "Shifter.h"
#include "Masker.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "ByteMask.h"
#include "SystemBus.h"
#include "BusRequestBuffer.h"

// This takes a byteindex/bytewrite/halfwrite to extract a byte/halfword from the incoming word and shift it appropriately.
// It also takes a wordoffset to shift the whole word across the cache line appropriately.
// It this maskes the word into the provided dataline

// wordwrite = false, linewrite=true -> return dataline, all 1s.
// wordwrite = true, linewrite=false -> return dataline?+dataword, mask mostly 0s with shifted 1s. Expected to be masked into the existing cache
// wordwrite = true, linewrite=true -> return dataline+dataword, all 1s, expect to be written to cache as is

template <unsigned int N>
class CacheLineMasker : public Component
{
public:
	typedef Bundle<N> CacheLine;
	void Connect(const Bundle<2> byteindex, const Bundle<bits(N)-5> wordoffset, const Bundle<32>& dataword, const CacheLine& dataline,
		const Wire& bytewrite, const Wire& halfwrite, const Wire& wordwrite, const Wire& linewrite)
	{
		maskedDataWord.Connect(byteindex, dataword, bytewrite, halfwrite, wordwrite);
		lineDataShifter.Connect(maskedDataWord.Word().ZeroExtend<N>(), wordoffset);
		lineMaskShifter.Connect(maskedDataWord.WordMask().ZeroExtend<N>(), wordoffset);
		maskedCacheLine.Connect(lineDataShifter.Out(), dataline, lineMaskShifter.Out());
		lineFullMask.Connect({ lineMaskShifter.Out(), CacheLine::ON }, linewrite);
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
	LeftShifterByWord<N, 32> lineDataShifter;
	LeftShifterByWord<N, 32> lineMaskShifter;
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

	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite, SystemBus & bus);
	void Update();
	void UpdateUntilNoStall(bool flush = false);

	const DataBundle& Out() const { return outDataMux.Out(); }
	const Wire& CacheHit() { return cacheHitMux.Out(); }
	const Wire& NeedStall() { return needStall.Out(); }
	const Wire& NoStall() { return needStallInv.Out(); }
	const Wire& PendingOps() { return busBuffer.Busy(); }

	
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

	OrGate readOrWrite;
	AndGateN<16> unCacheableAddr; // Replace this with a lookup table of some kind, or a tag!
	Inverter cacheableAddr;
	AndGate uncachedWrite;
	AndGate uncachedRead;

	CacheLineMasker<CACHE_LINE_BITS> lineWriteMasker;
	AndGate cachelinewrite;
	Decoder<NUM_CACHE_LINES> lineWriteDecoder;
	Decoder<NUM_CACHE_LINES> wordWriteDecoder;
	std::array<CacheLine<CACHE_LINE_BITS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Inverter readSuccessInv;
	AndGate pendingUncachedRead;
	Register<WORD_SIZE> uncachedBuffer;

	MuxBundle<TAG_BITS, NUM_CACHE_LINES> lineTagMux;
	
	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Multiplexer<NUM_CACHE_LINES> cacheDirtyMux;
	Inverter cacheMissInternal;
	AndGate cacheMiss;
	AndGate cacheableAddrCacheMiss;
	AndGate evictedDirty;
	Inverter notEvictedDirty;
	OrGate shouldSendWriteReq;
	OrGate shouldSendReadReq;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<CACHE_LINE_BITS, 2> outDataToBusMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outCacheDataMux;
	MuxBundle<WORD_SIZE, 2> outDataMux;
	MuxBundle<ADDR_BITS, 2> memAddrMux;

	BusRequestBuffer<CACHE_LINE_BITS, ADDR_BITS, 2> busBuffer;

	OrGate needStall;
	Inverter needStallInv;
	int cycles;

#if DEBUG
	CacheAddrBundle DEBUG_addr;
	DataBundle DEBUG_data;
	const Wire* DEBUG_read;
	const Wire* DEBUG_write;
#endif

	friend class Debugger;
};

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& read,
	const Wire& write, const Wire& bytewrite, const Wire& halfwrite, SystemBus & bus)
{
	CacheAddrBundle address(addr);
#if DEBUG
	DEBUG_addr = address;
	DEBUG_data = data;
	DEBUG_read = &read;
	DEBUG_write = &write;
#endif

	readOrWrite.Connect(read, write);

	// Determine if the address is cacheable
	unCacheableAddr.Connect(addr.Range<16>(16));
	cacheableAddr.Connect(unCacheableAddr.Out());
	uncachedWrite.Connect(unCacheableAddr.Out(), write);
	uncachedRead.Connect(unCacheableAddr.Out(), read);
		
	// Mask in the write word with the line we got from memory
	// This produces either the buffer read line with "data", if write is true, and a 32-bit shifted mask
	// Or just the buffer read line, if write is false, and a mask of all ones.
	lineWriteMasker.Connect(address.ByteIndex(), address.WordOffsetInLine(), data, busBuffer.OutRead(), bytewrite, halfwrite, write, busBuffer.ReadSuccess());

	// Temp Bundles for the cache line array
	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	std::array<TagBundle, NUM_CACHE_LINES> cacheLineTagOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;
	Bundle<NUM_CACHE_LINES> cacheDirtyCollector;
	
	cachelinewrite.Connect(cacheableAddr.Out(), busBuffer.ReadSuccess());
	CacheIndexBundle index = address.CacheLineIndex();
	lineWriteDecoder.Connect(index, cachelinewrite.Out());
	wordWriteDecoder.Connect(index, write);

	// Cache Lines
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(address.Tag(), lineWriteMasker.LineMask(), lineWriteMasker.Line(), lineWriteDecoder.Out()[i], wordWriteDecoder.Out()[i]);
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheLineTagOuts[i] = cachelines[i].Tag();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
		cacheDirtyCollector.Connect(i, cachelines[i].Dirty());
	}
	readSuccessInv.Connect(busBuffer.ReadSuccess());
	pendingUncachedRead.Connect(uncachedRead.Out(), readSuccessInv.Out());
	uncachedBuffer.Connect(busBuffer.OutRead().Range<32>(), uncachedRead.Out());

	// Evicted tag collector and addr calculation
	lineTagMux.Connect(cacheLineTagOuts, index);
	CacheAddrBundle evictedCacheAddr(addr);
	evictedCacheAddr.SetTag(lineTagMux.Out());

	// Status Flags
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheDirtyMux.Connect(cacheDirtyCollector, index);
	cacheMissInternal.Connect(cacheHitMux.Out());
	cacheMiss.Connect(cacheMissInternal.Out(), readOrWrite.Out());
	cacheableAddrCacheMiss.Connect(cacheMiss.Out(), cacheableAddr.Out());
	evictedDirty.Connect(cacheableAddrCacheMiss.Out(), cacheDirtyMux.Out());
	notEvictedDirty.Connect(evictedDirty.Out());
	shouldSendWriteReq.Connect(evictedDirty.Out(), uncachedWrite.Out());
	shouldSendReadReq.Connect(cacheableAddrCacheMiss.Out(), pendingUncachedRead.Out());

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
	memAddrMux.Connect({ addr, evictedCacheAddr }, evictedDirty.Out());

	busBuffer.Connect(bus, outDataToBusMux.Out(), memAddrMux.Out(), addr, shouldSendWriteReq.Out(), shouldSendReadReq.Out());
	
	// Stall the whole CPU pipeline if we have to.
	needStall.Connect(busBuffer.WriteFailed(), shouldSendReadReq.Out());
	needStallInv.Connect(needStall.Out());
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::Update()
{
	readOrWrite.Update();
#if DEBUG
	if (needStall.Out().On())
	{
		std::stringstream ss;
		ss << "Cache " << std::this_thread::get_id();
		ss << " is still stalling from the previous request (write buffered could mean it's a while)" << std::endl;
		std::cout << ss.str();
	}
	if (readOrWrite.Out().On())
	{
		std::cout << "Cache " << (DEBUG_read->On() ? "reading " : "writing ") << DEBUG_data.UnsignedRead() << " at " << DEBUG_addr.UnsignedRead() << std::endl;
	}
#endif

	unCacheableAddr.Update();
	cacheableAddr.Update();
	uncachedWrite.Update();
	uncachedRead.Update();

	busBuffer.PreUpdate();

	lineWriteMasker.Update();
	cachelinewrite.Update();
	lineWriteDecoder.Update();
	wordWriteDecoder.Update();

	// Must update the line tag mux *before* the registers, as their tags get stomped on write
	lineTagMux.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
	readSuccessInv.Update();
	pendingUncachedRead.Update();
	uncachedBuffer.Update();
	cacheHitMux.Update();
	cacheDirtyMux.Update();
	cacheMissInternal.Update();
	cacheMiss.Update();
	cacheableAddrCacheMiss.Update();
	evictedDirty.Update();
	notEvictedDirty.Update();

	shouldSendWriteReq.Update();
	shouldSendReadReq.Update();

	outCacheLineMux.Update();
	outDataToBusMux.Update();
	outCacheDataMux.Update();
	outDataMux.Update();
	memAddrMux.Update();

	busBuffer.Update();
	needStall.Update();
	needStallInv.Update();

	cycles++;
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS>::UpdateUntilNoStall(bool flush)
{
	do
	{
		Update();
	} while (NeedStall().On() || (flush && busBuffer.Busy().On()));
}