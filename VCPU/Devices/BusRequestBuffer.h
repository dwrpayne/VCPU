#pragma once
#include <array>

#include "Component.h"
#include "CircularBuffer.h"
#include "SystemBus.h"
#include "TriStateBuffer.h"
#include "ChangeDetector.h"
#include "EdgeDetector.h"

// This takes read and write requests.
// It buffers them before dropping them on the bus to main memory.
// It decides whether to send a read or write request.
// It informs out whether a wait for response is pending.
// Notifies when full in which case you need to wait for it to empty.
// Reads are blocking and take priority over writes.
// Read requests are served from the buffer.

template <unsigned int N, unsigned int Naddr, unsigned int Nbuf>
class BusRequestBuffer : public Component
{
public:
	typedef Bundle<N + Naddr> FullBundle;
	typedef Bundle<N> DataBundle;
	typedef Bundle<Naddr> AddrBundle;

	~BusRequestBuffer();
	void Connect(SystemBus& bus, const DataBundle& data, const AddrBundle& writeaddr, const AddrBundle& readaddr, const Wire& write, const Wire& read, int bus_master_num = 0);
	void PreUpdate();
	void Update();

	const DataBundle& OutRead() const { return readDataReg.Out(); }
#if DEBUG
	int WriteBufferCount() const { return (Nbuf + writeBuffer.size()) % Nbuf; }
#endif

	const Wire& WriteFailed() const { return writeFailed.Out(); }
	const Wire& WaitingForRead() const { return readRequested.Out(); }
	const Wire& ReadSuccess() const { return receivedReadAck.Out(); }
	const Wire& Busy() const { return busy.Out(); }

private:
	void ConnectToBus(SystemBus& bus);
	void DisconnectFromBus();

	SystemBus* pSystemBus;
	ChangeDetector<Naddr> newRead;
	ChangeDetector2<N, Naddr> newWrite;

	Inverter noAckOnBus;
	Inverter ackBuffer;
	AndGate receivedReadAck;
	Register<N> readDataReg;

	EdgeDetector grantedBus;
	AndGate shouldPopRead;
	AndGate shouldPopWrite;
	JKFlipFlop activeBusRequest;

	JKFlipFlop waitingForRead;
	OrGate readRequested;
	JKFlipFlop waitingForWrite;

	CircularBuffer<N + Naddr, Nbuf> writeBuffer;
	CircularBuffer<Naddr, 1> readBuffer;

	OrGate havePendingReadRequest;
	OrGate havePendingRequests;
	TriState shouldOutputOnDataBus;

	Inverter didNotWrite;
	AndGate writeFailed;
	OrGate busy;

	MuxBundle<Naddr, 2> addrMux;
	TriStateN<Naddr> addrRequestBuf;
	TriStateN<N> dataRequestBuf;
	TriState readRequestBuf;
	TriState writeRequestBuf;
	AndGate busRequestBuf;
	TriState requestBuf;

	AddrBundle DEBUG_addr;
	DataBundle DEBUG_data;

	int mBusMasterNum;
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
	const AddrBundle& writeaddr, const AddrBundle& readaddr, const Wire& write, const Wire& read, int bus_master_num)
{
	mBusMasterNum = bus_master_num;
	ConnectToBus(bus);
	DEBUG_addr = writeaddr;
	DEBUG_data = data;

	// Matchers to make sure we don't push the same request twice
	newRead.Connect(readaddr, read);
	newWrite.Connect(data, writeaddr, write);

	// Read data buffer
	noAckOnBus.Connect(pSystemBus->OutCtrl().Ack());
	ackBuffer.Connect(noAckOnBus.Out());
	receivedReadAck.Connect(waitingForRead.Q(), ackBuffer.Out());
	readDataReg.Connect(pSystemBus->OutData().Range<N>(), receivedReadAck.Out());

	// Decide if we should pop, and if so where. Only if we have been granted the bus
	// Only pop a read if we don't have any pending writes.
	grantedBus.Connect(pSystemBus->OutCtrl().BusGrant(bus_master_num));
	shouldPopRead.Connect(grantedBus.Rise(), writeBuffer.Empty());
	shouldPopWrite.Connect(grantedBus.Rise(), writeBuffer.NonEmpty());

	// If we were granted, we have an active request until we get ack. We can write to the data/addr lines and set REQ
	activeBusRequest.Connect(grantedBus.Rise(), ackBuffer.Out());

	// waitingFor -> "We are sending to the bus now and waiting on a response".
	waitingForRead.Connect(shouldPopRead.Out(), ackBuffer.Out());
	waitingForWrite.Connect(shouldPopWrite.Out(), ackBuffer.Out());

	// We have been asked to do a read and don't have a response yet.
	// There is a one cycle buffer delay to acquire the bus.
	readRequested.Connect(waitingForRead.Q(), newRead.Out());

	// Update the write buffer.
	FullBundle bundle;
	bundle.Connect(0, writeaddr);
	bundle.Connect(Naddr, data);
	writeBuffer.Connect(bundle, shouldPopWrite.Out(), newWrite.Out());

	// Update the read buffer.
	readBuffer.Connect(readaddr, shouldPopRead.Out(), newRead.Out());

	// Check if we can use the bus.
	havePendingReadRequest.Connect(readBuffer.NonEmpty(), waitingForRead.Q());
	havePendingRequests.Connect(writeBuffer.NonEmpty(), readBuffer.NonEmpty());
	shouldOutputOnDataBus.Connect(activeBusRequest.Q(), waitingForWrite.Q());
		
	// Actually acquire the bus.
	// If we have pending requests, we want to request the bus unless we already have it
	busRequestBuf.Connect(havePendingRequests.Out(), activeBusRequest.NotQ());
	
	// Status flags
	didNotWrite.Connect(writeBuffer.DidPush());
	writeFailed.Connect(didNotWrite.Out(), newWrite.Out());
	busy.Connect(activeBusRequest.Q(), havePendingRequests.Out());

	// Output buffers
	addrMux.Connect({ writeBuffer.Out().Range<Naddr>(), readBuffer.Out() }, waitingForRead.Q());
	addrRequestBuf.Connect(addrMux.Out(), activeBusRequest.Q());
	dataRequestBuf.Connect(writeBuffer.Out().Range<N>(Naddr), shouldOutputOnDataBus.Out());
	readRequestBuf.Connect(waitingForRead.Q(), activeBusRequest.Q());
	writeRequestBuf.Connect(waitingForWrite.Q(), shouldOutputOnDataBus.Out());
	requestBuf.Connect(activeBusRequest.Q(), Wire::ON);
	
	activeBusRequest.Update();
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::PreUpdate()
{
	noAckOnBus.Update();
	ackBuffer.Update();
	receivedReadAck.Update();
	readDataReg.Update();

#if DEBUG
	if (receivedReadAck.Out().On())
	{
		std::stringstream ss;
		ss << "Received a new read of ";
		readDataReg.Out().print(ss);
		ss << std::endl;
		std::cout << ss.str();
	}
#endif
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusRequestBuffer<N, Naddr, Nbuf>::Update()
{
	newRead.Update();
	newWrite.Update();

#if DEBUG
	if (newWrite.Out().On())
	{
		std::cout << "Requested a new write of " << DEBUG_data << " at " << DEBUG_addr << std::endl;
	}
#endif
	
	grantedBus.Update();
	shouldPopRead.Update();
	shouldPopWrite.Update();

	activeBusRequest.Update();

	waitingForRead.Update();
	waitingForWrite.Update();
	readRequested.Update();

	writeBuffer.Update();
	readBuffer.Update();

	havePendingReadRequest.Update();
	havePendingRequests.Update();
	shouldOutputOnDataBus.Update();

	busRequestBuf.Update();

	didNotWrite.Update();
	writeFailed.Update();
	busy.Update();


#if DEBUG
	if (haveBusOwnership.Q().On())
	{
		std::stringstream ss;
		ss << "Cache " << std::this_thread::get_id();
		ss << " has the bus for a " << (readRequestBuf.Out().On() ? "read" : (writeRequestBuf.Out().On() ? "write" : "hold"));
		ss << " at " << std::hex << addrRequestBuf.Out().UnsignedRead() << std::endl;
		std::cout << ss.str();
	}
	else if (busRequestBuf.Out().On())
	{
		std::stringstream ss;
		ss << "Cache " << std::this_thread::get_id();
		ss << " just released the bus. Request off. " << std::endl;
		std::cout << ss.str();
	}
#endif

	addrMux.Update();
	addrRequestBuf.Update();
	dataRequestBuf.Update();
	readRequestBuf.Update();
	writeRequestBuf.Update();
	requestBuf.Update();
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
	pSystemBus->ConnectCtrl(requestBuf.Out(), SystemBus::CtrlBit::Req);
	pSystemBus->ConnectCtrl(busRequestBuf.Out(), SystemBus::GetBusReq(mBusMasterNum));
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
		pSystemBus->DisconnectCtrl(requestBuf.Out(), SystemBus::CtrlBit::Req);
		pSystemBus->DisconnectCtrl(busRequestBuf.Out(), SystemBus::GetBusReq(mBusMasterNum));
	}
}