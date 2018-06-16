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
		const ActionBundle Action() const { ActionBundle a; a.Connect(0, Range<ACTION_LEN>(ADDR_LEN + DataBundle::N)); return a; }
	};

	void Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action);
	void Update();
	void UpdateRead(); // Hack until I can figure out how this can work.
	const BufBundle Out() { return buffer.Out(); }
	
private:
	OrGateN<ACTION_LEN> pushBuffer;
	Wire read, write;
	CircularBuffer<BUF_WIDTH, Nreg> buffer;
	std::mutex mMutex;
};

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void WriteBuffer<N, ADDR_LEN, Nreg>::Connect(const AddrBundle & addr, const DataBundle & data, const ActionBundle& action)
{
	pushBuffer.Connect(action);
	write.Set(false);
	read.Set(false);
	buffer.Connect(BufBundle(addr, data, action), read, write);
}

template <unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void WriteBuffer<N, ADDR_LEN, Nreg>::Update()
{
	std::unique_lock<std::mutex> lk(mMutex);
	pushBuffer.Update();
	write.Set(pushBuffer.Out().On());
	read.Set(false);
	buffer.Update();
}

template<unsigned int N, unsigned int ADDR_LEN, unsigned int Nreg>
inline void WriteBuffer<N, ADDR_LEN, Nreg>::UpdateRead()
{
	std::unique_lock<std::mutex> lk(mMutex);
	pushBuffer.Update();
	write.Set(false);
	read.Set(true);
	buffer.Update();
}
