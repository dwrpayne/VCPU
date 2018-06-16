#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"
#include "Register.h"
#include "Counter.h"
#include "MuxBundle.h"
#include "MultiGate.h"

template <unsigned int N, unsigned int Nreg>
class CircularBuffer : Component
{
public:
	static const int ADDR_LEN = bits(Nreg);
	typedef Bundle<ADDR_LEN> AddrBundle;
	
	void Connect(const Bundle<N>& in, const Wire & read, const Wire& write);
	void ConnectRead(const Wire& read);
	void Update();
	const Bundle<N>& Out() { return outMux.Out(); }

	const Wire& Full() { return counters.Full(); }
	const Wire& NonEmpty() { return counters.NonEmpty(); }

private:
	class CounterPair : public Component
	{
	public:
		void Connect(const Wire& read, const Wire& write);
		void Update();
		void UpdateCounters();
		const AddrBundle WriteIndex() { return writeCounter.Out().Range<ADDR_LEN>(); }
		const AddrBundle ReadIndex() { return readCounter.Out().Range<ADDR_LEN>(); }
		const Wire& Full() { return queueFull.Out(); }
		const Wire& HasRoom() { return queueNotFull.Out(); }
		const Wire& NonEmpty() { return queueNotEmpty.Out(); }

	private:
		Counter<ADDR_LEN + 1> readCounter;
		Counter<ADDR_LEN + 1> writeCounter;
		Matcher<ADDR_LEN> matcher;
		XorGate counterHiBitsDifferent;
		Inverter counterHiBitsEqual;
		AndGate queueFull;
		Inverter queueNotFull;
		NandGate queueNotEmpty;
	};

	CounterPair counters;
	AndGate writeEnable;
	AndGate readEnable;
	Decoder<Nreg> writeDecoder;
	MultiGate<AndGate, Nreg> bufferWriteEnable;
	std::array<Register<N>, Nreg> buffers;
	MuxBundle<N, Nreg> outMux;
};

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::Connect(const Bundle<N>& in, const Wire & read, const Wire & write)
{
	writeEnable.Connect(write, counters.HasRoom());
	readEnable.Connect(read, counters.NonEmpty());
	counters.Connect(readEnable.Out(), writeEnable.Out());
	writeDecoder.Connect(counters.WriteIndex());
	bufferWriteEnable.Connect(writeDecoder.Out(), Bundle<Nreg>(writeEnable.Out()));
	std::array<Bundle<N>, Nreg> regOuts;
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Connect(in, bufferWriteEnable.Out()[i]);
		regOuts[i].Connect(0, buffers[i].Out());
	}
	outMux.Connect(regOuts, counters.ReadIndex());

	counters.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::Update()
{
	writeEnable.Update();
	readEnable.Update();
	counters.Update();

	writeDecoder.Update();
	bufferWriteEnable.Update();
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Update();
	}
	outMux.Update();
	counters.UpdateCounters();
}

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::CounterPair::Connect(const Wire & read, const Wire & write)
{
	readCounter.Connect(Wire::OFF, read);
	writeCounter.Connect(Wire::OFF, write);
	matcher.Connect(readCounter.Out().Range<ADDR_LEN>(), writeCounter.Out().Range<ADDR_LEN>());
	counterHiBitsDifferent.Connect(readCounter.Out()[ADDR_LEN], writeCounter.Out()[ADDR_LEN]);
	counterHiBitsEqual.Connect(counterHiBitsDifferent.Out());
	queueFull.Connect(matcher.Out(), counterHiBitsDifferent.Out());
	queueNotFull.Connect(queueFull.Out());
	queueNotEmpty.Connect(matcher.Out(), counterHiBitsEqual.Out());
}

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::CounterPair::Update()
{
	readCounter.Update();
	writeCounter.Update();
	matcher.Update();
	counterHiBitsDifferent.Update();
	counterHiBitsEqual.Update();
	queueFull.Update();
	queueNotFull.Update();
	queueNotEmpty.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::CounterPair::UpdateCounters()
{
}
