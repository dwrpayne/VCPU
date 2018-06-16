#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"
#include "Register.h"
#include "Counter.h"
#include "MuxBundle.h"
#include "MultiGate.h"

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
class WriteBuffer : Component
{
public:
	static const int ACTION_LEN = 4;
	static const int BUF_WIDTH = N + ADDR_LEN + ACTION_LEN;
	static const int REG_INDEX_BITS = bits(Nreg);
	typedef Bundle<ADDR_LEN> AddrBundle;
	typedef Bundle<N> DataBundle;

	class ActionBundle : public Bundle<ACTION_LEN>
	{
	public:
		using Bundle<ACTION_LEN>::Bundle;
		const Wire& Write() const { return Get(0); }
		const Wire& WriteByte() const { return Get(1); }
		const Wire& WriteHalf() const { return Get(2); }
		const Wire& Read() const { return Get(3); }
	};

	class BufBundle : public Bundle<BUF_WIDTH>
	{
	public:
		using Bundle<BUF_WIDTH>::Bundle;
		BufBundle(const Bundle<BUF_WIDTH>& other)
			: Bundle<BUF_WIDTH>(other)
		{}
		BufBundle(const AddrBundle& addr, const DataBundle& data, const ActionBundle& action)
			: Bundle<BUF_WIDTH>()
		{
			Connect(0, addr);
			Connect(AddrBundle::N, data);
			Connect(AddrBundle::N + DataBundle::N, action);
		}

		const AddrBundle Addr() const { return Range<ADDR_LEN>(); }
		const DataBundle Data() const { return Range<DataBundle::N>(ADDR_LEN); }
		const Wire& Write() const { return Get(DataBundle::N + ADDR_LEN); }
		const Wire& WriteByte() const { return Get(DataBundle::N + ADDR_LEN + 1); }
		const Wire& WriteHalf() const { return Get(DataBundle::N + ADDR_LEN + 2); }
		const Wire& Read() const { return Get(DataBundle::N + ADDR_LEN + 3); }
	};

	void Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action, const Wire& read);
	void Update();
	const BufBundle Out() { return outMux.Out(); }

	const Bundle<REG_INDEX_BITS> WriteIndex() { return counterWrite.Out().Range<REG_INDEX_BITS>(); }
	const Bundle<REG_INDEX_BITS> ReadIndex() { return counterRead.Out().Range<REG_INDEX_BITS>(); }
	const Wire& Full() { return queueFull.Out(); }
	const Wire& NonEmpty() { return queueNotEmpty.Out(); }

private:
	class BufRegister : public Register<BUF_WIDTH>
	{
	public:
		void Connect(const AddrBundle& addr, const DataBundle& data, const ActionBundle& action, const Wire& enable)
		{
			Register<BUF_WIDTH>::Connect(BufBundle(addr, data, action), enable);
		}
		const BufBundle OutBuf() { return Out(); }
	};

	AndGateN<ACTION_LEN> pushBuffer;
	AndGate writeEnable;
	AndGate readEnable;
	Decoder<Nreg> writeDecoder;
	MultiGate<AndGate, Nreg> bufferWriteEnable;
	std::array<BufRegister, Nreg> buffers;
	Counter<REG_INDEX_BITS+1> counterWrite; // Extra bit for knowing if our buffer is full or empty (for Nreg=8: w=2, r=10 means full, w=2,r=2 means empty)
	Counter<REG_INDEX_BITS+1> counterRead;
	Matcher<REG_INDEX_BITS> counterMatch;
	XorGate counterHiBitsDifferent;
	Inverter counterHiBitsEqual;
	AndGate queueFull;
	Inverter queueNotFull;
	NandGate queueNotEmpty;

	MuxBundle<BUF_WIDTH, Nreg> outMux;
};

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void WriteBuffer<N, ADDR_LEN, Nreg>::Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action, const Wire& popBuffer)
{
	pushBuffer.Connect(action);
	counterMatch.Connect(counterRead.Out().Range<REG_INDEX_BITS>(), counterWrite.Out().Range<REG_INDEX_BITS>());
	counterHiBitsDifferent.Connect(counterRead.Out()[REG_INDEX_BITS], counterWrite.Out()[REG_INDEX_BITS]);
	counterHiBitsEqual.Connect(counterHiBitsDifferent.Out());
	queueFull.Connect(counterMatch.Out(), counterHiBitsDifferent.Out());
	queueNotFull.Connect(queueFull.Out());
	queueNotEmpty.Connect(counterMatch.Out(), counterHiBitsEqual.Out());

	writeEnable.Connect(pushBuffer.Out(), queueNotFull.Out());
	readEnable.Connect(popBuffer, queueNotEmpty.Out());

	writeDecoder.Connect(counterWrite.Out().Range<REG_INDEX_BITS>());
	bufferWriteEnable.Connect(writeDecoder.Out(), Bundle<Nreg>(writeEnable.Out()));

	std::array<Bundle<BUF_WIDTH>, Nreg> regOuts;
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Connect(addr, data, action, bufferWriteEnable.Out()[i]);
		regOuts[i].Connect(0, buffers[i].Out());
	}
	counterWrite.Connect(Wire::OFF, writeEnable.Out());
	counterRead.Connect(Wire::OFF, readEnable.Out());
	outMux.Connect(regOuts, counterRead.Out().Range<REG_INDEX_BITS>());
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void WriteBuffer<N, ADDR_LEN, Nreg>::Update()
{
	pushBuffer.Update();

	counterMatch.Update();
	counterHiBitsDifferent.Update();
	counterHiBitsEqual.Update();
	queueFull.Update();
	queueNotFull.Update();
	queueNotEmpty.Update();

	writeEnable.Update();
	readEnable.Update();

	writeDecoder.Update();
	bufferWriteEnable.Update();
	for (int i = 0; i < Nreg; i++)
	{
		buffers[i].Update();
	}
	counterWrite.Update();
	counterRead.Update();
	outMux.Update();
}
