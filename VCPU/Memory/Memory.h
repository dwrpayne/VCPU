#pragma once

#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "AndGate.h"
#include "MuxBundle.h"
#include "ThreadedComponent.h"
#include "SystemBus.h"
#include "BusSlave.h"

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
	const Wire& ServicedWrite() { return servicedWrite.Out(); }

private:
	BusSlaveConnector<N> busConnector;
	AndGate	reqBuffer;
	NorGateN<4> usercodeBusAddr;
	Inverter notAckOnBus;
	AndGate incomingRequest;
	AndGate servicedRead;
	AndGate servicedWrite;
	NorGate userdataBusAddr;
	Register<N> mDataFromBusBuffer;

	Decoder<NUM_LINES> addrDecoder;

	std::array<Register<N>, NUM_LINES> cachelines;
	MuxBundle<N, NUM_LINES> outMux;

	int cycle;

	MultiGate<AndGate, N> outData;
	DFlipFlop outServicedRequest;

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
	WaitUntilDone();
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Connect(SystemBus& bus)
{
	busConnector.Connect(bus, outData.Out(), outServicedRequest.Q());

	usercodeBusAddr.Connect(busConnector.GetAddr().Range<4>(-4));
	userdataBusAddr.Connect(usercodeBusAddr.Out(), busConnector.GetAddr().Range<1>(-1));
	if (mIsMainMemory)
	{
		incomingRequest.Connect(userdataBusAddr.Out(), busConnector.Request());
	}
	else
	{
		incomingRequest.Connect(usercodeBusAddr.Out(), busConnector.Request());
	}
	servicedRead.Connect(incomingRequest.Out(), busConnector.ReadRequest());
	servicedWrite.Connect(incomingRequest.Out(), busConnector.WriteRequest());
	mDataFromBusBuffer.Connect(busConnector.GetData(), servicedWrite.Out());

	auto cachelineAddr = busConnector.GetAddr().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);

	addrDecoder.Connect(cachelineAddr, servicedWrite.Out());
	
	auto* lineOuts = new std::array<CacheLineBundle, NUM_LINES>();

	for (int i = 0; i < NUM_LINES; ++i)
	{
		cachelines[i].Connect(mDataFromBusBuffer.Out(), addrDecoder.Out()[i]);
		lineOuts->at(i).Connect(0, cachelines[i].Out());
	}

	outMux.Connect(*lineOuts, cachelineAddr);

	outData.Connect(outMux.Out(), Bundle<N>(servicedRead.Out()));
	outServicedRequest.Connect(incomingRequest.Out(), Wire::ON);
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Update()
{
	cycle++;
	busConnector.Update();

	usercodeBusAddr.Update();
	userdataBusAddr.Update();
	incomingRequest.Update();
#if DEBUG
	if (incomingRequest.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " starting to service a " << (busConnector.ReadRequest().On() ? "read" : (busConnector.WriteRequest().On() ? "write" : "hold"));
		ss << " at " << std::hex << busConnector.GetAddr().UnsignedRead() << std::endl;
		std::cout << ss.str();
	}
#endif
	servicedRead.Update();
	servicedWrite.Update();
	mDataFromBusBuffer.Update();

#if DEBUG
	if (servicedWrite.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " writing at  " << std::hex << (int)(busConnector.GetAddr().UnsignedRead()/32)*32 << ", data: " << std::dec;
		busConnector.GetData().print(ss);
		ss << std::endl;
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
#if DEBUG
	if (incomingRequest.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " just finished memory " << (busConnector.ReadRequest().On() ? "read" : (busConnector.WriteRequest().On() ? "write" : "hold"));
		ss << " at " << std::hex << busConnector.GetAddr().UnsignedRead() << ". Ack on!" << std::endl;
		std::cout << ss.str();
	}
#endif
	outServicedRequest.Update();
	busConnector.PostUpdate();
}
