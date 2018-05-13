#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "MuxBundle.h"


template <unsigned int N, unsigned int NReg>
class RegisterFile : public Component
{
public:
	static const unsigned int ADDR_BITS = bits(NReg);
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef Bundle<N> DataBundle;

	void Connect(const AddrBundle& addr1, const AddrBundle& addr2, const DataBundle& data, const Wire& read, const Wire& write);
	void Update();

	const DataBundle& Out1() { return out1Enable.Out(); }
	const DataBundle& Out2() { return out2Enable.Out(); }

private:
	Decoder<NReg> addr1Decoder;
	MultiGate<AndGate, NReg> writeEnable;
	std::array<Register<N>, NReg> registers;

	MuxBundle<N, NReg> out1Mux, out2Mux;
	MultiGate<AndGate, N> out1Enable, out2Enable;
};

template<unsigned int N, unsigned int NReg>
inline void RegisterFile<N, NReg>::Connect(const AddrBundle & addr1, const AddrBundle & addr2, const DataBundle & data, const Wire& read, const Wire& write)
{
	addr1Decoder.Connect(addr1);
	writeEnable.Connect(addr1Decoder.Out(), Bundle<NReg>(write));

	std::array<DataBundle, NReg> regOuts;
	for (int i = 0; i < NReg; ++i)
	{
		registers[i].Connect(data, writeEnable.Out()[i]);
		regOuts[i] = registers[i].Out();
	}
	out1Mux.Connect(regOuts, addr1);
	out2Mux.Connect(regOuts, addr2);
	out1Enable.Connect(out1Mux.Out(), Bundle<N>(read));
	out2Enable.Connect(out2Mux.Out(), Bundle<N>(read));
}

template<unsigned int N, unsigned int NReg>
inline void RegisterFile<N, NReg>::Update()
{
	addr1Decoder.Update();
	writeEnable.Update();
	for (auto& reg : registers)
	{
		reg.Update();
	}
	out1Mux.Update();
	out2Mux.Update();
	out1Enable.Update();
	out2Enable.Update();
}
