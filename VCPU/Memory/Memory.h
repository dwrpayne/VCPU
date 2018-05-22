#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "Counter.h"
#include "Decoder.h"
#include "MuxBundle.h"

// Memory is stored in words (32 bit registers)
// But addressed by byte, so addresses take 2 bits more than the # of 32-bit registers

template <unsigned int N, unsigned int NReg, unsigned int NCacheLine=N*2>
class Memory : public Component
{
public:
	static const unsigned int WORD_LEN = N / 8;
	static const unsigned int BYTES = N * NReg * WORD_LEN;
	static const unsigned int ADDR_BITS = bits(NReg) + bits(WORD_LEN);
	static const unsigned int CACHE_WORDS = NCacheLine / N;
	static const unsigned int OFFSET_BITS = bits(CACHE_WORDS);
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<N> DataBundle;

	Memory();
	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write);
	void Update();

	const DataBundle& Out() { return outMux.Out(); }
	const Bundle<NCacheLine> OutLine() const { return outLine; }

private:
	Decoder<NReg> addrDecoder;
	MultiGate<AndGate, NReg> writeEnable;
	std::array<Register<N>, NReg> registers;
	MuxBundle<N, NReg> outMux;

	Counter<OFFSET_BITS> burstCounter;
	Decoder<CACHE_WORDS> counterDecoder;
	std::array<Register<N>, CACHE_WORDS> outBufferWords;
	Bundle<NCacheLine> outLine;

	friend class Debugger;
};

template<unsigned int N, unsigned int NReg, unsigned int NCacheLine>
inline Memory<N, NReg, NCacheLine>::Memory()
{
	for (int i = 0; i < CACHE_WORDS; ++i)
	{
		outLine.Connect(i*N, outBufferWords[i].Out());
	}
}

template<unsigned int N, unsigned int NReg, unsigned int NCacheLine = N>
inline void Memory<N, NReg, NCacheLine>::Connect(const AddrBundle & addr, const DataBundle & data, const Wire& write)
{
	auto byteAddr = addr.Range<bits(WORD_LEN)>(0);
	auto wordAddr = addr.Range<bits(NReg)>(bits(WORD_LEN));

	addrDecoder.Connect(wordAddr);
	writeEnable.Connect(addrDecoder.Out(), Bundle<NReg>(write));

	std::array<DataBundle, NReg> regOuts;
	for (int i = 0; i < NReg; ++i)
	{
		registers[i].Connect(data, writeEnable.Out()[i]);
		regOuts[i] = registers[i].Out();
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

template<unsigned int N, unsigned int NReg, unsigned int NCacheLine = N>
inline void Memory<N, NReg, NCacheLine>::Update()
{
	addrDecoder.Update();
	writeEnable.Update();
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
}
