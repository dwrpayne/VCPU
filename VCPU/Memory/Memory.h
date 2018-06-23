#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "Counter.h"
#include "AndGate.h"
#include "Decoder.h"
#include "MuxBundle.h"
#include "Shifter.h"
#include "SelectBundle.h"
#include "RequestBuffer.h"
#include "ThreadedComponent.h"
#include "SystemBusBuffer.h"

// Memory is read and written in cache lines
template <unsigned int N, unsigned int BYTES>
class Memory : public ThreadedAsyncComponent
{
public:
	static const unsigned int BYTES = BYTES;
	static const unsigned int CACHELINE_BYTES = N / 8;
	static const unsigned int ADDR_BITS = bits(BYTES);
	static const unsigned int CACHELINE_ADDR_BITS = bits(CACHELINE_BYTES);
	static const unsigned int NUM_LINES = BYTES / CACHELINE_BYTES;
	static const unsigned int CACHELINE_INDEX_LEN = bits(NUM_LINES);

	typedef Bundle<N> CacheLineBundle;
	typedef Bundle<ADDR_BITS> AddrBundle;

	Memory(bool ismain);
	~Memory();
	void Connect();
	void Update();
	void ConnectToBus(SystemBus& bus);

	void DisconnectFromBus(SystemBus & bus);


private:
	SystemBusBuffer busBuffer;
	AndGate incomingRequest;
	AndGate servicedRead;

	Decoder<NUM_LINES> addrDecoder;

	std::array<Register<N>, NUM_LINES> cachelines;
	MuxBundle<N, NUM_LINES> outMux;

	int cycle;

	MultiGate<AndGate, N> outData;
	MultiGate<AndGate, ADDR_BITS> outAddr;
	DFlipFlop outServicedRequest;

	std::mutex mMutex;
	bool mIsMainMemory;

	friend class Debugger;
};

template<unsigned int N, unsigned int BYTES>
inline Memory<N, BYTES>::Memory(bool ismain)
	: ThreadedAsyncComponent(ismain ? L"Main Memory Update" : L"Instruction Memory Update")
	, mIsMainMemory(ismain)
{
}
template<unsigned int N, unsigned int BYTES>
inline Memory<N, BYTES>::~Memory()
{
	DisconnectFromBus(*busBuffer.GetSystemBus());
}
//template<unsigned int N, unsigned int BYTES>
//inline Memory<N, BYTES>::Memory(bool ismain, std::mutex& mutex, std::condition_variable& cv, bool& ready, bool& exit)
//	: ThreadedComponent(mutex, cv, ready, exit)
//	,mIsMainMemory(main)
//{
//}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Connect()
{
	if (mIsMainMemory)
	{
		incomingRequest.Connect(busBuffer.UserData(), busBuffer.OutCtrl().Req());
	}
	else
	{
		incomingRequest.Connect(busBuffer.UserCode(), busBuffer.OutCtrl().Req());
	}
	servicedRead.Connect(incomingRequest.Out(), busBuffer.OutCtrl().Read());

	auto cachelineAddr = busBuffer.OutAddr().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);

	addrDecoder.Connect(cachelineAddr, busBuffer.OutCtrl().Write());
	
	std::array<CacheLineBundle, NUM_LINES> lineOuts;
	for (int i = 0; i < NUM_LINES; ++i)
	{
		cachelines[i].Connect(busBuffer.OutData(), addrDecoder.Out()[i]);
		lineOuts[i].Connect(0, cachelines[i].Out());
	}

	outMux.Connect(lineOuts, cachelineAddr);

	outData.Connect(outMux.Out(), Bundle<N>(servicedRead.Out()));
	outAddr.Connect(busBuffer.OutAddr().Range<ADDR_BITS>(), Bundle<ADDR_BITS>(servicedRead.Out()));
	outServicedRequest.Connect(incomingRequest.Out(), Wire::ON);
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Update()
{
	cycle++;
	busBuffer.Update();
	incomingRequest.Update();
	servicedRead.Update();
#if DEBUG
	if (incomingRequest.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem " : "Ins Mem ");
		ss << "servicing a " << (servicedRead.Out().On() ? "read" : (busBuffer.OutCtrl().Write().On() ? "write" : "hold"));
		ss << " at " << std::hex << busBuffer.OutAddr().UnsignedRead() << std::endl;
		std::cout << ss.str();
	}
#endif
	addrDecoder.Update();
	for (auto& reg : cachelines)
	{
		reg.Update();
	}
	outMux.Update();
	outData.Update();
	outAddr.Update();

	outServicedRequest.Update();
}

template<unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::ConnectToBus(SystemBus & bus)
{
	busBuffer.Connect(bus);

	bus.ConnectAddr(outAddr.Out());
	bus.ConnectData(outData.Out());
	bus.ConnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);
}


template<unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::DisconnectFromBus(SystemBus & bus)
{
	bus.DisconnectAddr(outAddr.Out());
	bus.DisconnectData(outData.Out());
	bus.DisconnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);
}
