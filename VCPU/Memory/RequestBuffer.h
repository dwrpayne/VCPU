#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Counter.h"
#include "MultiGate.h"
#include "CircularBuffer.h"

template <unsigned int N, unsigned int Nreg, unsigned int POP_EVERY>
class PulsedPopBuffer : Component
{
public:
	typedef Bundle<N> DataBundle;
	
	void Connect(const DataBundle& data, const Wire& trypush, const Wire& enablepop);
	void Update();
	const DataBundle Out() { return out.Out(); }
	const Wire& Popped() { return popped.Q(); }
	const Wire& Pushed() { return buffer.DidPush(); }

	const Wire& Full() { return buffer.Full(); }
	const Wire& NonEmpty() { return buffer.NonEmpty(); }
	const Wire& Empty() { return buffer.Empty(); }

private:
	ClockFreqSwitcher<POP_EVERY> popCycleCounter;
	Matcher<N> prevMatch;
	Register<N> previous;
	AndGate push;
	AndGate pop;
	CircularBuffer<N, Nreg> buffer;

	Register<N> out;
	DFlipFlop popped;
};

template <unsigned int N, unsigned int Nreg, unsigned int POP_EVERY>
inline void PulsedPopBuffer<N, Nreg, POP_EVERY>::Connect(const DataBundle & data, const Wire & trypush, const Wire& enablepop)
{
	popCycleCounter.Connect();
	prevMatch.Connect(previous.Out(), data);
	push.Connect(prevMatch.NoMatch(), trypush);
	pop.Connect(popCycleCounter.Pulse(), enablepop);
	buffer.Connect(data, pop.Out(), push.Out());

	previous.Connect(data, buffer.DidPush());
	out.Connect(buffer.Out(), buffer.DidPop());
	popped.Connect(buffer.DidPop(), popCycleCounter.Pulse());
}

template <unsigned int N, unsigned int Nreg, unsigned int POP_EVERY>
inline void PulsedPopBuffer<N, Nreg, POP_EVERY>::Update()
{ 
	popCycleCounter.Update();
	prevMatch.Update();
	push.Update();
	pop.Update();
	buffer.Update();

	previous.Update();
	out.Update();
	popped.Update();
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
class RequestBuffer : Component
{
public:
	static const int BUF_WIDTH = N + ADDR_LEN;
	typedef Bundle<ADDR_LEN> AddrBundle;
	typedef Bundle<N> DataBundle;
	typedef Bundle<BUF_WIDTH> WriteBufBundle;
	
	class WriteReqBundle : public WriteBufBundle
	{
	public:
		using WriteBufBundle::Bundle;
		WriteReqBundle(const AddrBundle& addr, const DataBundle& data)
			: WriteBufBundle()
		{
			Connect(0, addr);
			Connect(AddrBundle::N, data);
		}

		const AddrBundle Addr() const { return Range<ADDR_LEN>(); }
		const DataBundle Data() const { return Range<DataBundle::N>(ADDR_LEN); }
	};

	void Connect(const AddrBundle & readaddr, const AddrBundle & writeaddr, const DataBundle & data, const Wire& writereq, const Wire& readreq);
	void Update();
	const WriteReqBundle& OutWrite() { return static_cast<const WriteReqBundle&>(writeOut.Out()); }
	const AddrBundle& OutRead() { return readOut.Out(); }
	const Wire& PoppedWrite() { return writebuffer.Popped(); }
	const Wire& PoppedRead() { return readbuffer.Popped(); }
	const Wire& PoppedRequest() { return popped.Out(); }
	
	const Wire& WriteFull() { return writebuffer.Full(); }
	const Wire& WritePending() { return writebuffer.NonEmpty(); }
	const Wire& ReadPending() { return readbuffer.Full(); }
	
private:
	PulsedPopBuffer<BUF_WIDTH, Nreg, POP_EVERY> writebuffer;
	PulsedPopBuffer<ADDR_LEN, 1, POP_EVERY> readbuffer;

	OrGate popped;

	MultiGate<AndGate,BUF_WIDTH> writeOut;
	MultiGate<AndGate, ADDR_LEN> readOut;
};

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
inline void RequestBuffer<N, ADDR_LEN, Nreg, POP_EVERY>::Connect(const AddrBundle & readaddr, const AddrBundle & writeaddr, const DataBundle & data, const Wire& writereq, const Wire& readreq)
{	
	writebuffer.Connect(WriteReqBundle(writeaddr, data), writereq, readbuffer.Empty());
	readbuffer.Connect(readaddr, readreq, readbuffer.NonEmpty());

	popped.Connect(writebuffer.Popped(), readbuffer.Popped());
	
	writeOut.Connect(writebuffer.Out(), WriteBufBundle(writebuffer.Popped()));
	readOut.Connect(readbuffer.Out(), AddrBundle(readbuffer.Popped()));
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
inline void RequestBuffer<N, ADDR_LEN, Nreg, POP_EVERY>::Update()
{ 
	writebuffer.Update();
	readbuffer.Update();

	popped.Update();
		
	writeOut.Update();
	readOut.Update();
}