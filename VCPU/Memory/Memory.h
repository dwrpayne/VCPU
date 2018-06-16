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

// Memory is always read in words, but stored and addressed by byte.
// For byte-writes we have to keep an std::array of Register<8>, not Register<N>

template <unsigned int N, unsigned int BYTES, unsigned int NCacheLine=N*2>
class Memory : public Component
{
public:
	static const unsigned int WORD_LEN = N / 8;
	static const unsigned int ADDR_BITS = bits(BYTES);
	static const unsigned int ADDR_BYTE_LEN = bits(WORD_LEN);
	static const unsigned int NUM_WORDS = BYTES / WORD_LEN;
	static const unsigned int ADDR_WORD_LEN = bits(NUM_WORDS);
	static const unsigned int CACHE_WORDS = NCacheLine / N;
	static const unsigned int OFFSET_BITS = bits(CACHE_WORDS);
	static const unsigned int COUNTER_LEN = 3;
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<N> DataBundle;
	typedef RequestBuffer<N, ADDR_BITS, 8> ReqBuffer;

	Memory();
	void Connect(ReqBuffer& reqbuf);
	void Update();

	const DataBundle& Out() const { return outMux.Out(); }
	const Bundle<ADDR_BITS>& ReadAddr() const { return addrReadOrNull.Out(); }
	const Wire& ServicedRead() const { return servicedRead.Out(); }
	const Bundle<NCacheLine> OutLine() const { return outLine; }

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
	MuxBundle<N, NUM_WORDS> outMux;

	Counter<OFFSET_BITS> burstCounter;
	Decoder<CACHE_WORDS> counterDecoder;
	std::array<Register<N>, CACHE_WORDS> outBufferWords;
	Bundle<NCacheLine> outLine;

	friend class Debugger;
	 
};

template<unsigned int N, unsigned int BYTES, unsigned int NCacheLine>
inline Memory<N, BYTES, NCacheLine>::Memory()
{
	for (int i = 0; i < CACHE_WORDS; ++i)
	{
		outLine.Connect(i*N, outBufferWords[i].Out());
	}
}

template<unsigned int N, unsigned int BYTES, unsigned int NCacheLine = N>
inline void Memory<N, BYTES, NCacheLine>::Connect(ReqBuffer& reqbuf)
{
	pReqBuffer = &reqbuf;

	servicedRead.Connect(Wire::ON, pReqBuffer->PoppedRead());

	addrRWMux.Connect({ pReqBuffer->OutRead(), pReqBuffer->OutWrite().Addr()}, pReqBuffer->PoppedWrite());
	addrReadOrNull.Connect(addrRWMux.Out(), Bundle<ADDR_BITS>(pReqBuffer->PoppedWrite()));
	
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

	std::array<DataBundle, NUM_WORDS> regOuts;
	for (int i = 0; i < BYTES; ++i)
	{
		unsigned int bit = (i % 4) * 8;
		registers[i].Connect(dataByteShifter.Out().Range<8>(bit), writeEnable[i].Out());
		regOuts[i/4].Connect(bit, registers[i].Out());
	}
	burstCounter.Connect(Wire::OFF, Wire::ON);
	counterDecoder.Connect(burstCounter.Out());

	auto b = wordAddr;
	b.Connect(0, burstCounter.Out());

	outMux.Connect(regOuts, b);
	
	for (int i = 0; i < CACHE_WORDS; ++i)
	{
		outBufferWords[i].Connect(outMux.Out(), counterDecoder.Out()[i]);
		outLine.Connect(i*N, outBufferWords[i].Out());
	}
}

template<unsigned int N, unsigned int BYTES, unsigned int NCacheLine = N>
inline void Memory<N, BYTES, NCacheLine>::Update()
{
	pReqBuffer->UpdatePop();
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
	for (int i = 0; i < CACHE_WORDS; ++i)
	{
		burstCounter.Update();
		counterDecoder.Update();

		outMux.Update();
		for (auto& reg : outBufferWords)
		{
			reg.Update();
		}
	}
	addrReadOrNull.Update();

#ifdef DEBUG
	if (servicedRead.Out().On())
	{
		std::cout << "Memory put ";
		for (auto& reg : outBufferWords)
		{
			std::cout << reg.Out().Read() << ", ";
		}
		std::cout << " on " << addrReadOrNull.Out().Read() << std::endl;
	}
#endif
}
