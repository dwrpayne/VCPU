#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"
#include "Register.h"
#include "Counter.h"
#include "MuxBundle.h"
#include "MultiGate.h"
#include "CircularBuffer.h"

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
class RequestBuffer : Component
{
public:
	static const int ACTION_LEN = 3;
	static const int BUF_WIDTH = N + ADDR_LEN + ACTION_LEN;
	static const int REG_INDEX_BITS = bits(Nreg);
	typedef Bundle<ADDR_LEN> AddrBundle;
	typedef Bundle<N> DataBundle;
	typedef Bundle<BUF_WIDTH> WriteBufBundle;
	typedef Bundle<ACTION_LEN> ActionBundle;

	class WriteReqType : public ActionBundle
	{
	public:
		using ActionBundle::Bundle;
		const Wire& Write() const { return Get(0); }
		const Wire& WriteByte() const { return Get(1); }
		const Wire& WriteHalf() const { return Get(2); }
	};

	class WriteReqBundle : public WriteBufBundle
	{
	public:
		using Bundle<BUF_WIDTH>::Bundle;
		WriteReqBundle(const WriteBufBundle& other)
			: Bundle<BUF_WIDTH>(other)
		{}
		WriteReqBundle(const AddrBundle& addr, const DataBundle& data, const ActionBundle& action)
			: Bundle<BUF_WIDTH>()
		{
			Connect(0, addr);
			Connect(AddrBundle::N, data);
			Connect(AddrBundle::N + DataBundle::N, action);
		}

		const AddrBundle Addr() const { return Range<ADDR_LEN>(); }
		const DataBundle Data() const { return Range<DataBundle::N>(ADDR_LEN); }
		const WriteReqType Action() const { WriteReqType a; a.Connect(0, Range<ACTION_LEN>(ADDR_LEN + DataBundle::N)); return a; }
	};

	void Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action, const Wire& readreq);
	void Update();
	const WriteReqBundle OutWrite() { return writeOut.Out(); }
	const AddrBundle& OutRead() { return readOut.Out(); }
	const Wire& PoppedWrite() { return poppedWrite.Q(); }
	const Wire& PoppedRead() { return poppedRead.Q(); }
	
	const Wire& WriteFull() { return writebuffer.Full(); }
	const Wire& WritePending() { return writebuffer.NonEmpty(); }
	const Wire& ReadPending() { return readbuffer.Full(); }
	
private:
	ClockFreqSwitcher<POP_EVERY> popCycleCounter;

	OrGateN<ACTION_LEN> pushWrite;
	AndGate popWrite;
	AndGate pushRead;
	AndGate popRead;
	CircularBuffer<BUF_WIDTH, Nreg> writebuffer;
	CircularBuffer<ADDR_LEN, 1> readbuffer;

	RegisterReset<BUF_WIDTH> writeOut;
	RegisterReset<ADDR_LEN> readOut;
	DFlipFlop poppedWrite;
	DFlipFlop poppedRead;
};

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
inline void RequestBuffer<N, ADDR_LEN, Nreg, POP_EVERY>::Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action, const Wire& readreq)
{
	popCycleCounter.Connect();

	pushWrite.Connect(action);
	pushRead.Connect(Wire::ON, readreq);
	popRead.Connect(popCycleCounter.Pulse(), writebuffer.Empty());
	popWrite.Connect(popCycleCounter.Pulse(), writebuffer.NonEmpty());

	writebuffer.Connect(WriteReqBundle(addr, data, action), popWrite.Out(), pushWrite.Out());
	readbuffer.Connect(addr, popRead.Out(), pushRead.Out());

	writeOut.Connect(writebuffer.Out(), popWrite.Out(), popRead.Out());
	readOut.Connect(readbuffer.Out(), popRead.Out(), popWrite.Out());
	poppedWrite.Connect(popWrite.Out(), popCycleCounter.Pulse());
	poppedRead.Connect(popRead.Out(), popCycleCounter.Pulse());
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg, unsigned int POP_EVERY>
inline void RequestBuffer<N, ADDR_LEN, Nreg, POP_EVERY>::Update()
{
	popCycleCounter.Update();
	pushWrite.Update();
	pushRead.Update();
	popWrite.Update();
	popRead.Update();
	 
	writebuffer.Update();
	readbuffer.Update();
	
	writeOut.Update();
	readOut.Update();
	poppedWrite.Update();
	poppedRead.Update();
}
