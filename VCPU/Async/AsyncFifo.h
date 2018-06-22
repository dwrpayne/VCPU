#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"
#include "Register.h"
#include "GrayCounter.h"
#include "Matcher.h"
#include "MuxBundle.h"
#include "MultiGate.h"

template <unsigned int N, unsigned int Nreg>
class AsyncFifo : Component
{
public:
	static const int ADDR_LEN = bits(Nreg);
	typedef Bundle<ADDR_LEN> AddrBundle;

	void ConnectWrite(const Bundle<N>& in, const Wire& write);
	void ConnectRead(const Wire & read);
	void Update() {}
	void UpdateWrite();
	void UpdateRead();
	const Bundle<N>& Out() { return readMux.Out(); }

	const Wire& Full() { return full.Out(); }
	const Wire& NonEmpty() { return nonEmpty.Out(); }
	const Wire& Empty() { return empty.Out(); }
	const Wire& DidWrite() { return writeEnable.Out(); }
	const Wire& DidRead() { return readEnable.Out(); }

private:
	GrayCounter<ADDR_LEN + 1> writeCounter;
	GrayCounter<ADDR_LEN + 1> readCounter;
	Matcher<ADDR_LEN + 1> empty;
	Matcher<ADDR_LEN - 1> fullLoBits;
	MultiGate<XorGate,2> fullHiBits;
	AndGateN<3> full;
	Inverter notFull;
	Inverter notEmpty;

	AndGate writeEnable;
	AndGate readEnable;
	Decoder<Nreg> writeDecoder;
	std::array<Register<N>, Nreg> buffers;
	MuxBundle<N, Nreg> readMux;
};

template<unsigned int N, unsigned int Nreg>
inline void AsyncFifo<N, Nreg>::ConnectWrite(const Bundle<N>& in, const Wire& write)
{
	writeCounter.Connect(Wire::OFF, writeEnable.Out());
	fullLoBits.Connect(readCounter.OutGray().Range<ADDR_LEN-1>(), writeCounter.OutGray().Range<ADDR_LEN-1>());
	fullHiBits.Connect(readCounter.OutGray().Range<2>(ADDR_LEN - 1), writeCounter.OutGray().Range<2>(ADDR_LEN - 1));
	full.Connect({ &fullLoBits.Out(), &fullHiBits.Out()[0], &fullHiBits.Out()[1] });
	notFull.Connect(full.Out());

	writeEnable.Connect(write, notFull.Out());

	writeDecoder.Connect(writeCounter.OutBin().Range<ADDR_LEN>(), writeEnable.Out());
	std::array<Bundle<N>, Nreg> regOuts;
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Connect(in, writeDecoder.Out()[i]);
		regOuts[i].Connect(0, buffers[i].Out());
	}

	readMux.Connect(regOuts, readCounter.OutBin().Range<ADDR_LEN>());
	writeCounter.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void AsyncFifo<N, Nreg>::ConnectRead(const Wire & read)
{
	readCounter.Connect(Wire::OFF, readEnable.Out());
	empty.Connect(readCounter.OutGray(), writeCounter.OutGray());
	notEmpty.Connect(empty.Out());
	readEnable.Connect(read, notEmpty.Out());
	readCounter.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void AsyncFifo<N, Nreg>::UpdateWrite()
{
	writeEnable.Update();
	writeDecoder.Update();
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Update();
	}
	writeCounter.Update();
	fullLoBits.Update();
	fullHiBits.Update();
	full.Update();
	notFull.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void AsyncFifo<N, Nreg>::UpdateRead()
{
	readEnable.Update();
	readMux.Update();
	readCounter.Update();
	empty.Update();
	notEmpty.Update();
}