#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "Counter.h"
#include "AndGate.h"
#include "Decoder.h"
#include "MuxBundle.h"
#include "Shifter.h"
#include "SelectBundle.h"
#include "RequestBuffer.h"
#include "ThreadedComponent.h"

// Memory is stored in words
// We write in words, and read in cache lines

template <unsigned int N, unsigned int CACHE_LINE_BITS, unsigned int BYTES>
class Memory : public ThreadedComponent
{
public:
	static const unsigned int WORD_BYTES = N / 8;
	static const unsigned int CACHELINE_BYTES = CACHE_LINE_BITS / 8;
	static const unsigned int CACHELINE_WORDS = CACHELINE_BYTES / WORD_BYTES;
	static const unsigned int ADDR_BITS = bits(BYTES);
	static const unsigned int BYTE_INDEX_LEN = bits(WORD_BYTES);
	static const unsigned int NUM_WORDS = BYTES / WORD_BYTES;
	static const unsigned int WORD_INDEX_LEN = bits(NUM_WORDS);
	static const unsigned int CACHELINE_ADDR_BITS = bits(CACHELINE_BYTES);
	static const unsigned int NUM_LINES = BYTES / CACHELINE_BYTES;
	static const unsigned int CACHELINE_INDEX_LEN = bits(NUM_LINES);
	static const unsigned int MEMORY_UPDATE_RATIO = 8;
	static const unsigned int WRITE_BUFFER_LEN = 8;

	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<CACHE_LINE_BITS> CacheLineBundle;
	typedef RequestBuffer<N, ADDR_BITS, WRITE_BUFFER_LEN, MEMORY_UPDATE_RATIO> ReqBuffer;

	using ThreadedComponent::ThreadedComponent;

	void Connect(ReqBuffer& reqbuf);
	void Update();

	const AddrBundle& ReadAddr() const { return outAddr.Out(); }
	const Wire& ServicedRead() const { return outServicedRead.Out(); }
	const CacheLineBundle& OutLine() const { return outData.Out(); }

private:

	ReqBuffer* pReqBuffer;
	MuxBundle<ADDR_BITS, 2> addrRWMux;
	AndGate servicedRead;

	Decoder<NUM_WORDS> addrDecoder;

	std::array<Register<N>, NUM_WORDS> registers;
	MuxBundle<CACHE_LINE_BITS, NUM_LINES> outMux;

	int cycle;

	Register<CACHE_LINE_BITS> outData;
	Register<1> outServicedRead;
	Register<ADDR_BITS> outAddr;

	std::mutex mMutex;

	friend class Debugger;
};

template <unsigned int N, unsigned int CACHE_LINE_BITS, unsigned int BYTES>
inline void Memory<N, CACHE_LINE_BITS, BYTES>::Connect(ReqBuffer& reqbuf)
{
	pReqBuffer = &reqbuf;

	servicedRead.Connect(Wire::ON, pReqBuffer->PoppedRead());
	addrRWMux.Connect({ pReqBuffer->OutWrite().Addr(), pReqBuffer->OutRead()}, servicedRead.Out());

	auto wordAddr = addrRWMux.Out().Range<WORD_INDEX_LEN>(BYTE_INDEX_LEN);
	addrDecoder.Connect(wordAddr, pReqBuffer->PoppedWrite());
	
	std::array<CacheLineBundle, NUM_LINES> lineOuts;
	for (int i = 0; i < NUM_WORDS; ++i)
	{
		registers[i].Connect(pReqBuffer->OutWrite().Data(), addrDecoder.Out()[i]);
		unsigned int cache_line = i / CACHELINE_WORDS;
		unsigned int bit_line_offset = (i % CACHELINE_WORDS) * N;
		lineOuts[cache_line].Connect(bit_line_offset, registers[i].Out());
	}

	auto cachelineAddr = addrRWMux.Out().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);
	outMux.Connect(lineOuts, cachelineAddr);

	outData.Connect(outMux.Out(), Wire::ON);
	outServicedRead.Connect(Bundle<1>(servicedRead.Out()), Wire::ON);
	outAddr.Connect(addrRWMux.Out(), Wire::ON);
}

template <unsigned int N, unsigned int CACHE_LINE_BITS, unsigned int BYTES>
inline void Memory<N, CACHE_LINE_BITS, BYTES>::Update()
{
	cycle++;
	servicedRead.Update();
	addrRWMux.Update();
	addrDecoder.Update();
	for (auto& reg : registers)
	{
		reg.Update();
	}
	outMux.Update();

	// TODO: This doesn't solve the race condition where this data is accessed
	// by the Cache asyncronously. We need a similar situation to the RequestBuffer
	// or the CPU PipelineBuffers.
	outData.Update();
	outServicedRead.Update();
	outAddr.Update();
}
