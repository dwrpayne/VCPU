#pragma once
#include <array>
#include <mutex>

#include "Component.h"
#include "Bundle.h"
#include "XorGate.h"
#include "Register.h"
#include "Counter.h"
#include "MuxBundle.h"
#include "MultiGate.h"
#include "CircularBuffer.h"

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
class RequestBuffer : Component
{
public:
	static const int ACTION_LEN = 3;
	static const int BUF_WIDTH = N + ADDR_LEN + ACTION_LEN;
	static const int REG_INDEX_BITS = bits(Nreg);
	typedef Bundle<ADDR_LEN> AddrBundle;
	typedef Bundle<N> DataBundle;

	class WriteReqType : public Bundle<ACTION_LEN>
	{
	public:
		using Bundle<ACTION_LEN>::Bundle;
		const Wire& Write() const { return Get(0); }
		const Wire& WriteByte() const { return Get(1); }
		const Wire& WriteHalf() const { return Get(2); }
	};

	class WriteReqBundle : public Bundle<BUF_WIDTH>
	{
	public:
		using Bundle<BUF_WIDTH>::Bundle;
		WriteReqBundle(const Bundle<BUF_WIDTH>& other)
			: Bundle<BUF_WIDTH>(other)
		{}
		WriteReqBundle(const AddrBundle& addr, const DataBundle& data, const WriteReqType& action)
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

	void Connect(const AddrBundle & addr, const DataBundle & data, const WriteReqType& action, const Wire& readreq);
	void ConnectPop(const Wire& writereq, const Wire& readreq);
	void Update();
	void UpdatePop(); // Hack until I can figure out how this can work.
	const WriteReqBundle OutWrite() { return writebuffer.Out(); }
	const AddrBundle& OutRead() { return readbuffer.Out(); }

	const Wire& Full() { return writebuffer.Full(); }
	const Wire& NonEmpty() { return writebuffer.NonEmpty(); }
	
private:
	OrGateN<ACTION_LEN> pushWriteBuffer;
	AndGate popWriteBuffer;
	AndGate pushReadBuffer;
	AndGate popReadBuffer;
	Wire popwrite, pushwrite;
	Wire popread, pushread;
	CircularBuffer<BUF_WIDTH, Nreg> writebuffer;
	CircularBuffer<N, 1> readbuffer;
	std::mutex mMutex;
};

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void RequestBuffer<N, ADDR_LEN, Nreg>::Connect(const AddrBundle & addr, const DataBundle & data, const WriteReqType& action, const Wire& readreq)
{
	pushWriteBuffer.Connect(action);
	pushReadBuffer.Connect(Wire::ON, readreq);
	pushwrite.Set(false);
	popwrite.Set(false);
	pushread.Set(false);
	popread.Set(false);
	writebuffer.Connect(WriteReqBundle(addr, data, action), popwrite, pushwrite);
	readbuffer.Connect(addr, popread, pushread);
}

template<unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void RequestBuffer<N, ADDR_LEN, Nreg>::ConnectPop(const Wire& writereq, const Wire& readreq)
{
	popReadBuffer.Connect(Wire::ON, readreq);
	popWriteBuffer.Connect(Wire::ON, writereq);
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void RequestBuffer<N, ADDR_LEN, Nreg>::Update()
{
	std::unique_lock<std::mutex> lk(mMutex);
	pushWriteBuffer.Update();
	pushReadBuffer.Update();
	pushwrite.Set(pushWriteBuffer.Out().On());
	pushread.Set(pushReadBuffer.Out().On());
	popwrite.Set(false);
	popread.Set(false);
	writebuffer.Update();
	readbuffer.Update();
}

template<unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void RequestBuffer<N, ADDR_LEN, Nreg>::UpdatePop()
{
	std::unique_lock<std::mutex> lk(mMutex);
	popWriteBuffer.Update();
	popReadBuffer.Update();
	pushwrite.Set(false);
	pushread.Set(false);
	popwrite.Set(popWriteBuffer.Out().On());
	popread.Set(popReadBuffer.Out().On());
	writebuffer.Update();
	readbuffer.Update();
}
