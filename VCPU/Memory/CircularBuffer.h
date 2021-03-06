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

	void Connect(const Bundle<N>& in, const Wire & pop, const Wire& push);
	void Update();
	const Bundle<N>& Out()const { return popBuffer.Out(); }

	const Wire& Full() const { return counters.Full(); }
	const Wire& NonEmpty() const { return counters.NonEmpty(); }
	const Wire& Empty() const { return counters.Empty(); }
	const Wire& DidPush() const { return pushEnable.Out(); }
	const Wire& DidPop() const { return popEnable.Out(); }

#if DEBUG
	int size() const { return counters.WriteIndex().Read() - counters.ReadIndex().Read(); }
#endif

private:
	class CounterPair : public Component
	{
	public:
		void Connect(const Wire& read, const Wire& write);
		void Update();
		const AddrBundle WriteIndex() const { return writeCounter.Out().Range<ADDR_LEN>(); }
		const AddrBundle ReadIndex() const { return readCounter.Out().Range<ADDR_LEN>(); }
		const Wire& Full() const { return queueFull.Out(); }
		const Wire& HasRoom() const { return queueNotFull.Out(); }
		const Wire& NonEmpty() const { return queueNotEmpty.Out(); }
		const Wire& Empty() const { return queueEmpty.Out(); }

	private:
		Counter<ADDR_LEN + 1> readCounter;
		Counter<ADDR_LEN + 1> writeCounter;
		Matcher<ADDR_LEN> matcher;
		XorGate counterHiBitsDifferent;
		Inverter counterHiBitsEqual;
		AndGate queueFull;
		Inverter queueNotFull;
		AndGate queueEmpty;
		Inverter queueNotEmpty;
	};

	CounterPair counters;
	OrGate couldPush;
	AndGate pushEnable;
	OrGate couldPop;
	AndGate popEnable;
	Decoder<Nreg> writeDecoder;
	std::array<Register<N>, Nreg> buffers;
	MuxBundle<N, Nreg> outMux;
	MuxBundle<N, 2> forwardPushMux;
	Register<N> popBuffer;
};

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::Connect(const Bundle<N>& in, const Wire & pop, const Wire& push)
{
	couldPush.Connect(pop, counters.HasRoom());
	pushEnable.Connect(push, couldPush.Out());
	couldPop.Connect(pushEnable.Out(), counters.NonEmpty());
	popEnable.Connect(pop, couldPop.Out());
	counters.Connect(popEnable.Out(), pushEnable.Out());

	writeDecoder.Connect(counters.WriteIndex(), pushEnable.Out());
	std::array<Bundle<N>, Nreg> regOuts;
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Connect(in, writeDecoder.Out()[i]);
		regOuts[i].Connect(0, buffers[i].Out());
	}

	outMux.Connect(regOuts, counters.ReadIndex());
	forwardPushMux.Connect({ outMux.Out(), in }, counters.Empty());
	popBuffer.Connect(forwardPushMux.Out(), popEnable.Out());

	counters.Update();
}

template<unsigned int N, unsigned int Nreg>
inline void CircularBuffer<N, Nreg>::Update()
{
	couldPush.Update();
	pushEnable.Update();
	couldPop.Update();
	popEnable.Update();

	writeDecoder.Update();
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Update();
	}
	forwardPushMux.Update();
	popBuffer.Update();
	counters.Update();
	outMux.Update();

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
	queueEmpty.Connect(matcher.Out(), counterHiBitsEqual.Out());
	queueNotEmpty.Connect(queueEmpty.Out());
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
	queueEmpty.Update();
	queueNotEmpty.Update();
}
