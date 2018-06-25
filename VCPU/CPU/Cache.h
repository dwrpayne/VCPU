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
	}
	void Update()
	{
		maskedDataWord.Update();
		lineDataShifter.Update();
		lineMaskShifter.Update();
		maskedCacheLine.Update();
	}
	const Bundle<32>& WordMask() const { return maskedDataWord.WordMask(); }
	const Bundle<32>& Word() const { return maskedDataWord.Word(); }
	const CacheLine& Line() const { return maskedCacheLine.Out(); }

private:
	WordMasker maskedDataWord;
	LeftShifter<N> lineDataShifter;
	LeftShifter<N> lineMaskShifter;
	Masker<N> maskedCacheLine;
};


template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES = 2048>
class Cache : public Component
{
public:
	static const int WORD_SIZE = 32;
	static const int ADDR_BITS = 32;
	static const int CACHE_BYTES = CACHE_SIZE_BYTES;
	static const int MEMORY_BYTES = MAIN_MEMORY_BYTES;
	static const int CACHE_LINE_BYTES = CACHE_LINE_BITS / 8;


	static const int WORD_BYTES = WORD_SIZE / 8;
	static const int BYTE_INDEX_BITS = bits(WORD_BYTES);
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
	typedef RequestBuffer<CACHE_LINE_BITS, ADDR_BITS, 4, 4> ReqBuffer;

	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write, const Wire& read, const Wire& bytewrite, const Wire& halfwrite);
	void ConnectToBus(SystemBus& bus);
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
	std::array<CacheLine<WORD_SIZE, CACHE_WORDS, TAG_BITS>, NUM_CACHE_LINES> cachelines;

	Matcher<ADDR_BITS> addrReadMatcher;
	AndGateN<3> gotResultFromMemory;

	Decoder<NUM_CACHE_LINES> indexDecoder;

	CacheLineMasker<CACHE_LINE_BITS> lineWriteMasker;
	
	Multiplexer<NUM_CACHE_LINES> cacheHitMux;
	Multiplexer<NUM_CACHE_LINES> cacheDirtyMux;
	AndGate evictedDirty;
	Inverter notEvictedDirty;
	Inverter cacheMissInternal;
	OrGate readOrWrite;
	AndGate cacheMiss;
	AndGate writeBufferFull;
	OrGate needStall;
	Inverter needStallInv;
	AndGate readOkay;

	MuxBundle<CACHE_LINE_BITS, NUM_CACHE_LINES> outCacheLineMux;
	MuxBundle<WORD_SIZE, CACHE_WORDS> outDataMux;

	MuxBundle<TAG_BITS, NUM_CACHE_LINES> lineTagMux;
	MuxBundle<ADDR_BITS, 4> memAddrMux;

	NorGate busIsFree;
	OrGate busIsFreeOrMine;
	Inverter busReqNotYetAck;
	AndGate shouldOutputOnBus;
	AndGate shouldOutputDataBus;
	
	TriState writeBusRequestBuf;
	TriState readBusRequestBuf;
	TriState busRequestBuf;
	MultiGate<TriState, CACHE_LINE_BITS> dataRequestBuf;
	DFlipFlop haveBusOwnership;

	int cycles;

	std::mutex mBusMutex;

#ifdef DEBUG
	CacheAddrBundle DEBUG_addr;
#endif

	friend class Debugger;
};

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Connect(const AddrBundle& addr, const DataBundle& data, const Wire& read,
	const Wire& write, const Wire& bytewrite, const Wire& halfwrite)
{
	CacheAddrBundle address(addr);
#ifdef DEBUG
	DEBUG_addr = address;
#endif

	CacheIndexBundle index = address.CacheLineIndex();
		
	// Did we get data from memory. Mask it in with the data we want to write.
	gotResultFromMemory.Connect({ &haveBusOwnership.Q(), &cacheMiss.Out(), &pSystemBus->OutCtrl().Ack() });
	indexDecoder.Connect(index, Wire::ON);
	lineWriteMasker.Connect(address.ByteIndex(), address.WordOffsetInLine(), data, pSystemBus->OutData(), bytewrite, halfwrite, write);

	// Temp Bundles for the cache line array
	std::array<Bundle<CACHE_LINE_BITS>, NUM_CACHE_LINES> cacheLineDataOuts;
	std::array<TagBundle, NUM_CACHE_LINES> cacheLineTagOuts;
	Bundle<NUM_CACHE_LINES> cacheHitCollector;
	Bundle<NUM_CACHE_LINES> cacheDirtyCollector;

	// Cache Lines
	for (int i = 0; i < NUM_CACHE_LINES; ++i)
	{
		cachelines[i].Connect(address.Tag(), address.WordOffsetInLine(), lineWriteMasker.WordMask(), lineWriteMasker.Word(), 
			gotResultFromMemory.Out(), lineWriteMasker.Line(), indexDecoder.Out()[i], write);
		cacheLineDataOuts[i] = cachelines[i].OutLine();
		cacheLineTagOuts[i] = cachelines[i].Tag();
		cacheHitCollector.Connect(i, cachelines[i].CacheHit());
		cacheDirtyCollector.Connect(i, cachelines[i].Dirty());
	}

	// Evicted tag collector and addr calculation
	lineTagMux.Connect(cacheLineTagOuts, index);
	CacheAddrBundle evictedCacheAddr(addr);
	evictedCacheAddr.SetTag(lineTagMux.Out());
	memAddrMux.Connect({ AddrBundle::OFF, evictedCacheAddr, addr, evictedCacheAddr }, { &evictedDirty.Out(), &cacheMiss.Out() });

	// Status Flags
	cacheHitMux.Connect(cacheHitCollector, index);
	cacheDirtyMux.Connect(cacheDirtyCollector, index);
	cacheMissInternal.Connect(cacheHitMux.Out());
	readOrWrite.Connect(read, write);
	cacheMiss.Connect(cacheMissInternal.Out(), readOrWrite.Out());
	evictedDirty.Connect(cacheMiss.Out(), cacheDirtyMux.Out());
	notEvictedDirty.Connect(evictedDirty.Out());
	writeBufferFull.Connect(evictedDirty.Out(), Wire::ON);
	needStall.Connect(cacheMiss.Out(), writeBufferFull.Out());
	needStallInv.Connect(needStall.Out());
	readOkay.Connect(cacheMiss.Out(), notEvictedDirty.Out());

	// Output 
	outCacheLineMux.Connect(cacheLineDataOuts, index);
	std::array<DataBundle, CACHE_WORDS> dataWordBundles;
	for (int i = 0; i < CACHE_WORDS; i++)
	{
		dataWordBundles[i] = outCacheLineMux.Out().Range<WORD_SIZE>(i*WORD_SIZE);
	}
	outDataMux.Connect(dataWordBundles, address.WordOffsetInLine());

	busIsFree.Connect(pSystemBus->OutCtrl().BusReq(), pSystemBus->OutCtrl().Ack());
	busIsFreeOrMine.Connect(haveBusOwnership.Q(), busIsFree.Out());
	haveBusOwnership.Connect(needStall.Out(), busIsFreeOrMine.Out());

	busReqNotYetAck.Connect(pSystemBus->OutCtrl().Ack());

	shouldOutputOnBus.Connect(haveBusOwnership.Q(), Wire::ON);
	shouldOutputDataBus.Connect(shouldOutputOnBus.Out(), write);

	writeBusRequestBuf.Connect(evictedDirty.Out(), shouldOutputOnBus.Out());
	readBusRequestBuf.Connect(readOkay.Out(), shouldOutputOnBus.Out());
	dataRequestBuf.Connect(outCacheLineMux.Out(), Bundle<CACHE_LINE_BITS>(shouldOutputDataBus.Out()));
	busRequestBuf.Connect(haveBusOwnership.Q(), shouldOutputOnBus.Out());
	
	//buffer.Connect(addr, memWriteAddrMux.Out(), outCacheLineMux.Out(), evictedDirty.Out(), cacheMiss.Out());
}

template <unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::Update()
{
	gotResultFromMemory.Update();
	indexDecoder.Update();
	lineWriteMasker.Update();
		
	// Must update the line tag mux *before* the registers, as their tags get stomped on write
	lineTagMux.Update();

	for (auto& line : cachelines)
	{
		line.Update();
	}
	cacheHitMux.Update();
	cacheDirtyMux.Update();
	cacheMissInternal.Update();
	readOrWrite.Update();
	cacheMiss.Update();
	evictedDirty.Update();
	notEvictedDirty.Update();
	writeBufferFull.Update();
	needStall.Update();
	needStallInv.Update();

	// We do write-before-read because writes aren't buffered to the bus yet.
	// This needs to change to a read first, write buffer, query the buffer first on cache miss, architecture.
	readOkay.Update();

	outCacheLineMux.Update();
	outDataMux.Update();
	memAddrMux.Update();
	//buffer.Update();
	{
		std::scoped_lock lk(mBusMutex);
		busIsFree.Update();
		busIsFreeOrMine.Update();
		haveBusOwnership.Update();
	}
	busReqNotYetAck.Update();
	shouldOutputOnBus.Update();
	shouldOutputDataBus.Update();
	writeBusRequestBuf.Update();
	readBusRequestBuf.Update();
	dataRequestBuf.Update();
	busRequestBuf.Update();
	
#if DEBUG
	if (haveBusOwnership.Q().On())
	{
		std::stringstream ss;
		ss << std::this_thread::get_id() << " requesting a " << (readBusRequestBuf.Out().On() ? "read" : (writeBusRequestBuf.Out().On() ? "write" : "hold"));
		ss << " at " << std::hex << memAddrMux.Out().UnsignedRead() << std::endl;
		std::cout << ss.str();
	}
#endif

	cycles++;
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::UpdateUntilNoStall(bool flush)
{
	do
	{
		Update();
	} while (NeedStall().On());// || (flush && buffer.WritePending().On()));
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::DisconnectFromBus()
{
	pSystemBus->DisconnectAddr(memAddrMux.Out());
	pSystemBus->DisconnectAddr(dataRequestBuf.Out());
	pSystemBus->DisconnectAddr(readBusRequestBuf.Out(), SystemBus::CtrlBit::Read);
	pSystemBus->DisconnectAddr(writeBusRequestBuf.Out(), SystemBus::CtrlBit::Write);
	pSystemBus->DisconnectAddr(busRequestBuf.Out(), SystemBus::CtrlBit::Req);
	pSystemBus->DisconnectAddr(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
}

template<unsigned int CACHE_SIZE_BYTES, unsigned int CACHE_LINE_BITS, unsigned int MAIN_MEMORY_BYTES>
inline void Cache<CACHE_SIZE_BYTES, CACHE_LINE_BITS, MAIN_MEMORY_BYTES>::ConnectToBus(SystemBus & bus)
{
	pSystemBus = &bus;
	pSystemBus->ConnectAddr(memAddrMux.Out());
	pSystemBus->ConnectData(dataRequestBuf.Out());
	pSystemBus->ConnectCtrl(readBusRequestBuf.Out(), SystemBus::CtrlBit::Read);
	pSystemBus->ConnectCtrl(writeBusRequestBuf.Out(), SystemBus::CtrlBit::Write);
	pSystemBus->ConnectCtrl(busRequestBuf.Out(), SystemBus::CtrlBit::Req);
	pSystemBus->ConnectCtrl(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
}
