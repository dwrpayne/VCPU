#pragma once
#include <array>

#include "Component.h"
#include "CircularBuffer.h"
#include "SystemBus.h"
#include "TriStateBuffer.h"
#include "Matcher.h"
#include "EdgeDetector.h"

// This takes read and write requests.
// It buffers them before dropping them on the bus to main memory.
// It decides whether to send a read or write request.
// It informs out whether a wait for response is pending.
// Notifies when full in which case you need to wait for it to empty.
// Reads are blocking and take priority over writes.
// Read requests are served from the buffer.

// Only allowed to read or write, not both at same time.

template <unsigned int N, unsigned int Naddr, unsigned int Nbuf>
class BusRequestBuffer : Component
{
public:
	typedef Bundle<N + Naddr> FullBundle;
	typedef Bundle<N> DataBundle;
	typedef Bundle<Naddr> AddrBundle;

	~BusRequestBuffer();
	void Connect(SystemBus& bus, const DataBundle& data, const AddrBundle& writeaddr, const AddrBundle& readaddr, const Wire& write, const Wire& read);
	void Update();

	const DataBundle& OutRead() const { return readDataReg.Out(); }
#if DEBUG
	int WriteBufferCount() const { return (Nbuf + writeBuffer.size()) % Nbuf; }
#endif

	const Wire& PendingRead() const { return waitingForRead.Q(); }
	const Wire& WriteSuccess() const { return writeBuffer.DidPush(); }
	const Wire& ReadSuccess() const { return receivedReadAck.Out(); }

private:
	void ConnectToBus(SystemBus& bus);
	void DisconnectFromBus();

	SystemBus* pSystemBus;
	Matcher<N> lastMatchData;
	Matcher<Naddr> lastMatchWriteAddr;
	Matcher<Naddr> lastMatchReadAddr;
	Register<N> lastWriteData;
	Register<Naddr> lastWriteAddr;
	Register<Naddr> lastReadAddr;
	OrGate newWriteValue;
	AndGate newWrite;
	AndGate newRead;

	Inverter noAckOnBus;
	Inverter ackBuffer;
	AndGate receivedReadAck;
	Register<N> readDataReg;

	EdgeDetector shouldPopEdge;
	AndGate shouldPopRead;
	AndGate shouldPopWrite;

	JKFlipFlop waitingForRead;
	JKFlipFlop waitingForWrite;

	CircularBuffer<N + Naddr, Nbuf> writeBuffer;
	CircularBuffer<Naddr, 1> readBuffer;

	TriState shouldOutputOnBus;
	TriState shouldOutputOnDataBus;
	OrGate havePendingRequests;

	NorGateN<3> busIsFree;
	OrGate busIsFreeOrMine;
	AndGate wantTakeBus;
	AndGate wantReleaseBus;
	JKFlipFlop haveBusOwnership;

	MuxBundle<Naddr, 2> addrMux;
	TriStateN<Naddr> addrRequestBuf;
	TriStateN<N> dataRequestBuf;
	TriState readRequestBuf;
	TriState writeRequestBuf;
	TriState busRequestBuf;
};


// Flush writes before read? Query is an optimization.
// Case: write buffer empty
//		Read - add to buf, get bus, pop, indicate wait, return data once arrived.
//		Write - add to buf, get bus, pop, return done.
// Case: write buffer half-full
//		Read - flush, then do empty.
//		Write - add to buf, continue as normal.
// Case: write buffer full
//		Read - flush, then do empty read.
//		Write - try add, realize full, signal full, continue as normal.

// If want to pop: 
//		Only pop read if write buf empty.
//		Otherwise pop write if it's non-empty


template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::Connect(SystemBus& bus, const DataBundle& data, 
	const AddrBundle& writeaddr, const AddrBundle& readaddr, const Wire& write, const Wire& read)
{
	ConnectToBus(bus);
#if DEBUG
	assert(!(read.On() && write.On()));
#endif
	// Matchers to make sure we don't push the same request twice
	lastMatchData.Connect(data, lastWriteData.Out());
	lastMatchWriteAddr.Connect(writeaddr, lastWriteAddr.Out());
	lastMatchReadAddr.Connect(readaddr, lastReadAddr.Out());
	lastWriteData.Connect(data, write);
	lastWriteAddr.Connect(writeaddr, write);
	lastReadAddr.Connect(readaddr, read);
	newWriteValue.Connect(lastMatchData.NoMatch(), lastMatchWriteAddr.NoMatch());
	newWrite.Connect(newWriteValue.Out(), write);
	newRead.Connect(read, lastMatchReadAddr.NoMatch());

	// Read data buffer
	noAckOnBus.Connect(pSystemBus->OutCtrl().Ack());
	ackBuffer.Connect(noAckOnBus.Out());
	receivedReadAck.Connect(waitingForRead.Q(), ackBuffer.Out());
	readDataReg.Connect(pSystemBus->OutData().Range<N>(), receivedReadAck.Out());

	// Decide if we should pop, and if so where. Only if we have bus ownership already.
	// Only pop a read if we don't have any pending writes.
	shouldPopEdge.Connect(haveBusOwnership.Q());
	shouldPopRead.Connect(shouldPopEdge.Rise(), writeBuffer.Empty());
	shouldPopWrite.Connect(shouldPopEdge.Rise(), writeBuffer.NonEmpty());

	waitingForRead.Connect(shouldPopRead.Out(), ackBuffer.Out());
	waitingForWrite.Connect(shouldPopWrite.Out(), ackBuffer.Out());

	// Update the write buffer.
	FullBundle bundle;
	bundle.Connect(0, writeaddr);
	bundle.Connect(Naddr, data);
	writeBuffer.Connect(bundle, shouldPopWrite.Out(), newWrite.Out());

	// Update the read buffer.
	readBuffer.Connect(readaddr, shouldPopRead.Out(), newRead.Out());

	// Check if we can use the bus.
	havePendingRequests.Connect(writeBuffer.NonEmpty(), readBuffer.NonEmpty());
	shouldOutputOnBus.Connect(noAckOnBus.Out(), haveBusOwnership.Q());
	shouldOutputOnDataBus.Connect(shouldOutputOnBus.Out(), waitingForWrite.Q());
		
	// Actually acquire the bus.
	busIsFree.Connect({&pSystemBus->OutCtrl().BusReq(), &ackBuffer.Out(), &pSystemBus->OutCtrl().Ack()});
	busIsFreeOrMine.Connect(haveBusOwnership.Q(), busIsFree.Out());
	wantTakeBus.Connect(busIsFree.Out(), havePendingRequests.Out());
	wantReleaseBus.Connect(haveBusOwnership.Q(), ackBuffer.Out());
	haveBusOwnership.Connect(wantTakeBus.Out(), wantReleaseBus.Out()); 

	// Output buffers
	addrMux.Connect({ writeBuffer.Out().Range<Naddr>(), readBuffer.Out() }, waitingForRead.Q());
	addrRequestBuf.Connect(addrMux.Out(), shouldOutputOnBus.Out());
	dataRequestBuf.Connect(writeBuffer.Out().Range<N>(Naddr), shouldOutputOnDataBus.Out());
	readRequestBuf.Connect(waitingForRead.Q(), shouldOutputOnBus.Out());
	writeRequestBuf.Connect(waitingForWrite.Q(), shouldOutputOnDataBus.Out());
	busRequestBuf.Connect(Wire::ON, shouldOutputOnBus.Out());	
	
	haveBusOwnership.Update();
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::Update()
{
	lastMatchData.Update();
	lastMatchWriteAddr.Update();
	lastMatchReadAddr.Update();
	lastWriteData.Update();
	lastWriteAddr.Update();
	lastReadAddr.Update();
	newWriteValue.Update();
	newWrite.Update();
	newRead.Update();
	
	noAckOnBus.Update();
	ackBuffer.Update();
	receivedReadAck.Update();
	readDataReg.Update();

	shouldPopEdge.Update();
	shouldPopRead.Update();
	shouldPopWrite.Update();

	waitingForRead.Update();
	waitingForWrite.Update();

	writeBuffer.Update();
	readBuffer.Update();

	havePendingRequests.Update();
	shouldOutputOnBus.Update();
	shouldOutputOnDataBus.Update();

	{
		std::scoped_lock lk(pSystemBus->mBusMutex);
		busIsFree.Update();
		busIsFreeOrMine.Update();
		wantTakeBus.Update();
		wantReleaseBus.Update();
		haveBusOwnership.Update();
	}

	addrMux.Update();
	addrRequestBuf.Update();
	dataRequestBuf.Update();
	readRequestBuf.Update();
	writeRequestBuf.Update();
	busRequestBuf.Update();
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline BusRequestBuffer<N, Naddr, Nbuf>::~BusRequestBuffer()
{
	DisconnectFromBus();
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::ConnectToBus(SystemBus& bus)
{
	pSystemBus = &bus;
	pSystemBus->ConnectAddr(addrRequestBuf.Out());
	pSystemBus->ConnectData(dataRequestBuf.Out());
	pSystemBus->ConnectCtrl(readRequestBuf.Out(), SystemBus::CtrlBit::Read);
	pSystemBus->ConnectCtrl(writeRequestBuf.Out(), SystemBus::CtrlBit::Write);
	pSystemBus->ConnectCtrl(shouldOutputOnBus.Out(), SystemBus::CtrlBit::Req);
	pSystemBus->ConnectCtrl(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::DisconnectFromBus()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectAddr(addrRequestBuf.Out());
		pSystemBus->DisconnectData(dataRequestBuf.Out());
		pSystemBus->DisconnectCtrl(readRequestBuf.Out(), SystemBus::CtrlBit::Read);
		pSystemBus->DisconnectCtrl(writeRequestBuf.Out(), SystemBus::CtrlBit::Write);
		pSystemBus->DisconnectCtrl(shouldOutputOnBus.Out(), SystemBus::CtrlBit::Req);
		pSystemBus->DisconnectCtrl(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
	}
}