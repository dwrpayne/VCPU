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

// Memory is stored in bytes, read in cache lines.
// We support byte writes, half word writes, and full word (32-bit) writes

template <unsigned int BYTES, unsigned int NCacheLine=N*2>
class Memory : public ThreadedComponent
{
public:
	static const unsigned int N = 32;
	static const unsigned int WORD_LEN = 4;
	static const unsigned int ADDR_BITS = bits(BYTES);
	static const unsigned int ADDR_BYTE_LEN = 2;
	static const unsigned int NUM_WORDS = BYTES / 4;
	static const unsigned int ADDR_WORD_LEN = bits(NUM_WORDS);
	static const unsigned int CACHELINE_WORDS = NCacheLine / 32;
	static const unsigned int CACHELINE_BYTES = NCacheLine / 8;
	static const unsigned int CACHELINE_ADDR_BITS = bits(NCacheLine);
	static const unsigned int NUM_LINES = BYTES / CACHELINE_BYTES;
	static const unsigned int CACHELINE_INDEX_LEN = bits(NUM_LINES);
	static const unsigned int OFFSET_BITS = bits(CACHELINE_WORDS);
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<NCacheLine> CacheLineBundle;
	typedef RequestBuffer<32, ADDR_BITS, 8, 8> ReqBuffer;
	
	using ThreadedComponent::ThreadedComponent;

	void Connect(ReqBuffer& reqbuf);
	void Update();

	const AddrBundle& ReadAddr() const { return addrReadOrNull.Out(); }
	const Wire& ServicedRead() const { return servicedRead.Out(); }
	const CacheLineBundle& OutLine() const { return outMux.Out(); }

private:

	ReqBuffer* pReqBuffer;
	MuxBundle<ADDR_BITS, 2> addrRWMux;
	MultiGate<AndGate, ADDR_BITS> addrReadOrNull;
	AndGate servicedRead;
	
	Decoder<NUM_WORDS> addrDecoder;
	Decoder<WORD_LEN> byteDecoder;
	Decoder<2> halfDecoder;
	MuxBundle<WORD_LEN, 4> masker;
	MultiGate<AndGate, WORD_LEN> writeEnabledMask;
	MuxBundle<WORD_LEN, 2> writeEnableMaskOrReadMux;
	LeftShifter<N> dataByteShifter;
	
	std::array<AndGate, BYTES> writeEnable;
	std::array<Register<8>, BYTES> registers;
	MuxBundle<NCacheLine, NUM_LINES> outMux;

	int cycle;

	friend class Debugger;	 
};


template<unsigned int BYTES, unsigned int NCacheLine = N>
inline void Memory<BYTES, NCacheLine>::Connect(ReqBuffer& reqbuf)
{
	pReqBuffer = &reqbuf;

	servicedRead.Connect(Wire::ON, pReqBuffer->PoppedRead());

	addrRWMux.Connect({ pReqBuffer->OutRead(), pReqBuffer->OutWrite().Addr() }, pReqBuffer->PoppedWrite());
	addrReadOrNull.Connect(addrRWMux.Out(), Bundle<ADDR_BITS>(pReqBuffer->PoppedRead()));

	auto byteAddr = addrRWMux.Out().Range<ADDR_BYTE_LEN>(0);
	auto wordAddr = addrRWMux.Out().Range<ADDR_WORD_LEN>(ADDR_BYTE_LEN);
	addrDecoder.Connect(wordAddr);
	byteDecoder.Connect(byteAddr);
	halfDecoder.Connect(Bundle<1>(byteAddr[1]));

	Bundle<4> halfMaskBundle({ &halfDecoder.Out()[0], &halfDecoder.Out()[0], &halfDecoder.Out()[1], &halfDecoder.Out()[1] });
	masker.Connect({ Bundle<4>::ON, byteDecoder.Out(), halfMaskBundle, Bundle<4>::ON },
		{ &pReqBuffer->OutWrite().Action().WriteByte(), &pReqBuffer->OutWrite().Action().WriteHalf() });
	writeEnabledMask.Connect(masker.Out(), Bundle<WORD_LEN>(pReqBuffer->OutWrite().Action().Write()));

	writeEnableMaskOrReadMux.Connect({ Bundle<WORD_LEN>::OFF, writeEnabledMask.Out() }, pReqBuffer->PoppedWrite());

	for (int i = 0; i < BYTES; ++i)
	{
		writeEnable[i].Connect(addrDecoder.Out()[i / 4], writeEnableMaskOrReadMux.Out()[i % 4]);
	}

	dataByteShifter.Connect(pReqBuffer->OutWrite().Data(), { &Wire::OFF, &Wire::OFF, &Wire::OFF, &byteAddr[0], &byteAddr[1] });

	std::array<CacheLineBundle, NUM_LINES> lineOuts;
	for (int i = 0; i < BYTES; ++i)
	{
		unsigned int bit_offset = (i % 4) * 8;
		registers[i].Connect(dataByteShifter.Out().Range<8>(bit_offset), writeEnable[i].Out());
		unsigned int cache_line = i / CACHELINE_BYTES;
		unsigned int bit_line_offset = (i % CACHELINE_BYTES) * 8;
		lineOuts[cache_line].Connect(bit_line_offset, registers[i].Out());
	}

	auto cachelineAddr = addrRWMux.Out().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);
	outMux.Connect(lineOuts, cachelineAddr);
}

template<unsigned int BYTES, unsigned int NCacheLine = N>
inline void Memory<BYTES, NCacheLine>::Update()
{
	cycle++;
	servicedRead.Update();
	addrRWMux.Update();
	addrDecoder.Update();
	byteDecoder.Update();
	halfDecoder.Update();
	masker.Update();
	writeEnabledMask.Update();
	writeEnableMaskOrReadMux.Update();
	dataByteShifter.Update();
	for (auto& i : writeEnable)
	{
		i.Update();
	}
	for (auto& reg : registers)
	{
		reg.Update();
	}
	outMux.Update();
	addrReadOrNull.Update();
}
