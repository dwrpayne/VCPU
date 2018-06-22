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

// Memory is read and written in cache lines
template <unsigned int N, unsigned int BYTES>
class Memory : public ThreadedComponent
{
public:
	static const unsigned int CACHELINE_BYTES = N / 8;
	static const unsigned int ADDR_BITS = bits(BYTES);
	static const unsigned int CACHELINE_ADDR_BITS = bits(CACHELINE_BYTES);
	static const unsigned int NUM_LINES = BYTES / CACHELINE_BYTES;
	static const unsigned int CACHELINE_INDEX_LEN = bits(NUM_LINES);
	static const unsigned int MEMORY_UPDATE_RATIO = 8;
	static const unsigned int WRITE_BUFFER_LEN = 4;

	typedef Bundle<N> CacheLineBundle;
	typedef Bundle<ADDR_BITS> AddrBundle;
	typedef RequestBuffer<N, ADDR_BITS, WRITE_BUFFER_LEN, MEMORY_UPDATE_RATIO> ReqBuffer;

	using ThreadedComponent::ThreadedComponent;

	void Connect(const AddrBundle& readaddr, const AddrBundle& writeaddr, const CacheLineBundle& data, const Wire& read, const Wire& write);
	void Update();
	void PostUpdate();

	const AddrBundle& ReadAddr() const { return outAddr.Out(); }
	const Wire& ServicedRead() const { return outServicedRead.Out(); }
	const CacheLineBundle& OutLine() const { return outData.Out(); }

private:

	MuxBundle<ADDR_BITS, 2> addrRWMux;
	AndGate servicedRead;

	Decoder<NUM_LINES> addrDecoder;

	std::array<Register<N>, NUM_LINES> cachelines;
	MuxBundle<N, NUM_LINES> outMux;

	int cycle;

	Register<N> outData;
	Register<1> outServicedRead;
	Register<ADDR_BITS> outAddr;

	std::mutex mMutex;

	friend class Debugger;
};

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Connect(const AddrBundle& readaddr, const AddrBundle& writeaddr, const CacheLineBundle& data, const Wire& read, const Wire& write)
{
	servicedRead.Connect(Wire::ON, read);
	addrRWMux.Connect({ writeaddr, readaddr}, servicedRead.Out());
	auto cachelineAddr = addrRWMux.Out().Range<CACHELINE_INDEX_LEN>(CACHELINE_ADDR_BITS);

	addrDecoder.Connect(cachelineAddr, write);
	
	std::array<CacheLineBundle, NUM_LINES> lineOuts;
	for (int i = 0; i < NUM_LINES; ++i)
	{
		cachelines[i].Connect(data, addrDecoder.Out()[i]);
		lineOuts[i].Connect(0, cachelines[i].Out());
	}

	outMux.Connect(lineOuts, cachelineAddr);

	outData.Connect(outMux.Out(), Wire::ON);
	outServicedRead.Connect(Bundle<1>(servicedRead.Out()), Wire::ON);
	outAddr.Connect(addrRWMux.Out(), Wire::ON);
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::Update()
{
	cycle++;
	servicedRead.Update();
	addrRWMux.Update();
	addrDecoder.Update();
	for (auto& reg : cachelines)
	{
		reg.Update();
	}
	outMux.Update();
}

template <unsigned int N, unsigned int BYTES>
inline void Memory<N, BYTES>::PostUpdate()
{
	outData.Update();
	outServicedRead.Update();
	outAddr.Update();
}
