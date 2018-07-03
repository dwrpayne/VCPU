#pragma once

#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "AndGate.h"
#include "MuxBundle.h"
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
	AndGate	reqBuffer;
	NorGateN<4> usercodeBusAddr;
	Inverter notAckOnBus;
	AndGateN<3> incomingRequest;
	AndGate servicedRead;
	AndGate servicedWrite;
	NorGate userdataBusAddr;

	Decoder<NUM_LINES> addrDecoder;

	std::array<Register<N>, NUM_LINES> cachelines;
	MuxBundle<N, NUM_LINES> outMux;

	int cycle;

	MultiGate<AndGate, N> outData;
	DFlipFlop outServicedRequest;

	std::mutex mMutex;
	bool mIsMainMemory;

	// A bit of a hack to enable writes to code-only memory.
	bool mIsLoadingProgram;

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
	DisconnectFromBus();
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Connect(SystemBus& bus)
{
	pSystemBus = &bus;

	pSystemBus->ConnectData(outData.Out());
	pSystemBus->ConnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);

	reqBuffer.Connect(pSystemBus->OutCtrl().Req(), Wire::ON);

	usercodeBusAddr.Connect(pSystemBus->OutAddr().Range<4>(-4));
	userdataBusAddr.Connect(usercodeBusAddr.Out(), pSystemBus->OutAddr().Range<1>(-1));
	notAckOnBus.Connect(pSystemBus->OutCtrl().Ack());
	if (mIsMainMemory)
	{
		incomingRequest.Connect({ &userdataBusAddr.Out(), &reqBuffer.Out(), &notAckOnBus.Out() });
	}
	else
	{
		incomingRequest.Connect({ &usercodeBusAddr.Out(), &reqBuffer.Out(), &notAckOnBus.Out() });
	}
	servicedRead.Connect(incomingRequest.Out(), pSystemBus->OutCtrl().Read());
	servicedWrite.Connect(incomingRequest.Out(), pSystemBus->OutCtrl().Write());

	auto cachelineAddr = pSystemBus->OutAddr().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);

	addrDecoder.Connect(cachelineAddr, servicedWrite.Out());
	
	auto* lineOuts = new std::array<CacheLineBundle, NUM_LINES>();

	for (int i = 0; i < NUM_LINES; ++i)
	{
		unsigned int line_bit_index = (NUM_LINES * N) % pSystemBus->Ndata;
		cachelines[i].Connect(pSystemBus->OutData().Range<N>(line_bit_index), addrDecoder.Out()[i]);
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
	reqBuffer.Update();

	usercodeBusAddr.Update();
	userdataBusAddr.Update();
	notAckOnBus.Update();
	incomingRequest.Update();
#if DEBUG
	if (incomingRequest.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " starting to service a " << (pSystemBus->OutCtrl().Read().On() ? "read" : (pSystemBus->OutCtrl().Write().On() ? "write" : "hold"));
		ss << " at " << std::hex << pSystemBus->OutAddr().UnsignedRead() << std::endl;
		std::cout << ss.str();
	}
#endif
	servicedRead.Update();
	servicedWrite.Update();

	if (servicedWrite.Out().On())
	{
		if(!(mIsLoadingProgram || mIsMainMemory))
			pSystemBus->PrintBus();
		assert((mIsLoadingProgram || mIsMainMemory) && "Attempting to write to code memory");
	}
#if DEBUG
	if (servicedWrite.Out().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " writing at  " << std::hex << (int)(pSystemBus->OutAddr().UnsignedRead()/32)*32 << ", data: " << std::dec;
		pSystemBus->OutData().print(ss);
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
		ss << " just finished memory " << (pSystemBus->OutCtrl().Read().On() ? "read" : (pSystemBus->OutCtrl().Write().On() ? "write" : "hold"));
		ss << " at " << std::hex << pSystemBus->OutAddr().UnsignedRead() << ". Ack on!" << std::endl;
		std::cout << ss.str();
	}
	else if (outServicedRequest.Q().On())
	{
		std::stringstream ss;
		ss << (mIsMainMemory ? "Main Mem" : "Ins Mem");
		ss << " finished servicing a " << (servicedRead.Out().On() ? "read" : "write") <<  ". Ack off: " << std::endl;
		std::cout << ss.str();
	}
#endif
	outServicedRequest.Update();

}

template<unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::DisconnectFromBus()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(outData.Out());
		pSystemBus->DisconnectCtrl(outServicedRequest.Q(), SystemBus::CtrlBit::Ack);
	}
}
