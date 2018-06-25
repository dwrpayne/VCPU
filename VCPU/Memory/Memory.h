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
#include "SystemBus.h"

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
	void Connect(SystemBus & bus);
	void Update();

private:
	void DisconnectFromBus();
	SystemBus * pSystemBus;
	AndGate incomingRequest;
	AndGate servicedRead;
	NorGateN<4> usercodeBusAddr;
	NorGate userdataBusAddr;

	Decoder<NUM_LINES> addrDecoder;

	std::array<Register<N>, NUM_LINES> cachelines;
	MuxBundle<N, NUM_LINES> outMux;

	int cycle;

	MultiGate<AndGate, N> outData;
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
	DisconnectFromBus();
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Connect(SystemBus& bus)
{
	pSystemBus = &bus;

	pSystemBus->ConnectData(outData.Out());
	pSystemBus->ConnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);

	usercodeBusAddr.Connect(pSystemBus->OutAddr().Range<4>(-4));
	userdataBusAddr.Connect(usercodeBusAddr.Out(), pSystemBus->OutAddr().Range<1>(-1));
	if (mIsMainMemory)
	{
		incomingRequest.Connect(userdataBusAddr.Out(), pSystemBus->OutCtrl().Req());
	}
	else
	{
		incomingRequest.Connect(usercodeBusAddr.Out(), pSystemBus->OutCtrl().Req());
	}
	servicedRead.Connect(incomingRequest.Out(), pSystemBus->OutCtrl().Read());

	auto cachelineAddr = pSystemBus->OutAddr().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);

	addrDecoder.Connect(cachelineAddr, pSystemBus->OutCtrl().Write());
	
	std::array<CacheLineBundle, NUM_LINES> lineOuts;
	for (int i = 0; i < NUM_LINES; ++i)
	{
		cachelines[i].Connect(pSystemBus->OutData(), addrDecoder.Out()[i]);
		lineOuts[i].Connect(0, cachelines[i].Out());
	}

	outMux.Connect(lineOuts, cachelineAddr);

	outData.Connect(outMux.Out(), Bundle<N>(servicedRead.Out()));
	outServicedRequest.Connect(incomingRequest.Out(), Wire::ON);
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Update()
{
	cycle++;
	usercodeBusAddr.Update();
	userdataBusAddr.Update();
	incomingRequest.Update();
	servicedRead.Update();
#if DEBUG
	if (incomingRequest.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem " : "Ins Mem ");
		ss << "servicing a " << (servicedRead.Out().On() ? "read" : (pSystemBus->OutCtrl().Write().On() ? "write" : "hold"));
		ss << " at " << std::hex << pSystemBus->OutAddr().UnsignedRead() << std::endl;
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

	outServicedRequest.Update();
}

template<unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::DisconnectFromBus()
{
	pSystemBus->DisconnectData(outData.Out());
	pSystemBus->DisconnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);
}
