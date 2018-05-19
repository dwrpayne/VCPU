#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"

// Memory is stored in words (32 bit registers)
// But addressed by byte, so addresses take 2 bits more than the # of 32-bit registers

template <unsigned int N, unsigned int NReg>
class Memory : public Component
{
public:
	static const unsigned int WORD_LEN = N / 8;
	static const unsigned int BYTES = N * NReg * WORD_LEN;
	static const unsigned int ADDR_BITS = bits(NReg) + bits(WORD_LEN);
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<N> DataBundle;

	void Connect(const AddrBundle& addr, const DataBundle& data, const Wire& write);
	void Update();

	const DataBundle& Out() { return outMux.Out(); }

private:
	Decoder<NReg> addrDecoder;
	MultiGate<AndGate, NReg> writeEnable;
	std::array<Register<N>, NReg> registers;
	MuxBundle<N, NReg> outMux;

	friend class Debugger;
};

template<unsigned int N, unsigned int NReg>
inline void Memory<N, NReg>::Connect(const AddrBundle & addr, const DataBundle & data, const Wire& write)
{
	auto byteAddr = addr.Range<bits(WORD_LEN)>(0);
	auto wordAddr = addr.Range<ADDR_BITS - bits(WORD_LEN)>(bits(WORD_LEN));

	addrDecoder.Connect(wordAddr);
	writeEnable.Connect(addrDecoder.Out(), Bundle<NReg>(write));

	std::array<DataBundle, NReg> regOuts;
	for (int i = 0; i < NReg; ++i)
	{
		registers[i].Connect(data, writeEnable.Out()[i]);
		regOuts[i] = registers[i].Out();
	}
	outMux.Connect(regOuts, wordAddr);
}

template<unsigned int N, unsigned int NReg>
inline void Memory<N, NReg>::Update()
{
	addrDecoder.Update();
	writeEnable.Update();
	for (auto& reg : registers)
	{
		reg.Update();
	}
	outMux.Update();
}
