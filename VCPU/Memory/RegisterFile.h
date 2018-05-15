#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"

// Note, this register file has R0 hardwired to all 0 bits, always.

template <unsigned int N, unsigned int NReg>
class RegisterFile : public Component
{
public:
	static const unsigned int ADDR_BITS = bits(NReg);
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<N> DataBundle;

	void Connect(const AddrBundle& addr1, const AddrBundle& addr2, const AddrBundle& addrw, const DataBundle& data, const Wire& write);
	void Update();

	const DataBundle& Out1() { return out1Mux.Out(); }
	const DataBundle& Out2() { return out2Mux.Out(); }

private:
	Decoder<NReg> addrwDecoder;
	MultiGate<AndGate, NReg> writeEnable;
	std::array<Register<N>, NReg> registers; // Make space for R0

	MuxBundle<N, NReg> out1Mux, out2Mux;
};

template<unsigned int N, unsigned int NReg>
inline void RegisterFile<N, NReg>::Connect(const AddrBundle & addr1, const AddrBundle & addr2, const AddrBundle& addrw, const DataBundle & data, const Wire& write)
{
	addrwDecoder.Connect(addrw);
	writeEnable.Connect(addrwDecoder.Out(), Bundle<NReg>(write));

	std::array<DataBundle, NReg> regOuts;
	registers[0].Connect(DataBundle(Wire::OFF), Wire::OFF);
	regOuts[0] = registers[0].Out();
	for (int i = 1; i < NReg; ++i)
	{
		registers[i].Connect(data, writeEnable.Out()[i]);
		regOuts[i] = registers[i].Out();
	}
	out1Mux.Connect(regOuts, addr1);
	out2Mux.Connect(regOuts, addr2);
}

template<unsigned int N, unsigned int NReg>
inline void RegisterFile<N, NReg>::Update()
{
	addrwDecoder.Update();
	writeEnable.Update();
	for (auto& reg : registers)
	{
		reg.Update();
	}
	out1Mux.Update();
	out2Mux.Update();
}
