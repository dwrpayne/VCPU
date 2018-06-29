#pragma once
#include <array>

#include "Component.h"
#include "CircularBuffer.h"
#include "SystemBus.h"
#include "TriStateBuffer.h"
#include "Matcher.h"
#include "EdgeDetector.h"

// This buffers writes to the bus. They are fire-and-forget and you don't get a result back.
// Used by the cache to send writes off to main memory.
// Notifies when full in which case you need to wait for it to empty.

template <unsigned int N, unsigned int Naddr, unsigned int Nbuf>
class BusWriteBuffer : Component
{
public:
	typedef Bundle<N + Naddr> FullBundle;
	void Connect(SystemBus& bus, const Bundle<N>& data, const Bundle<Naddr>& addr, const Wire& write);
	void Update();

	const Wire& Full() { return circbuf.Full(); }
	const Wire& NonEmpty() { return circbuf.NonEmpty(); }
	const Wire& Empty() { return circbuf.Empty(); }
	const Wire& DidWrite() { return circbuf.DidPush(); }

private:
	void ConnectToBus(SystemBus& bus);
	void DisconnectFromBus();

	SystemBus* pSystemBus;
	Matcher<N> lastMatchData;
	Matcher<Naddr> lastMatchAddr;
	Register<N> lastWriteData;
	Register<Naddr> lastWriteAddr;
	OrGate newWrite;

	EdgeDetector shouldPopEdge;
	CircularBuffer<N+Naddr, Nbuf> circbuf;
	
	NorGate busIsFree;
	OrGate busIsFreeOrMine;
	AndGate wantTakeBus;
	AndGate wantReleaseBus;
	JKFlipFlop haveBusOwnership;

	TriState shouldOutputOnBus;
	TriStateN<Naddr> addrRequestBuf;
	TriStateN<N> dataRequestBuf;
	TriState busRequestBuf;
};

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusWriteBuffer<N, Naddr, Nbuf>::Connect(SystemBus& bus, const Bundle<N>& data, const Bundle<Naddr>& addr, const Wire& write)
{
	ConnectToBus(bus);
	lastWriteData.Connect(data, write);
	lastMatchData.Connect(data, lastWriteData.Out());
	lastWriteAddr.Connect(addr, write);
	lastMatchAddr.Connect(addr, lastWriteAddr.Out());
	newWrite.Connect(lastMatchData.NoMatch(), lastMatchAddr.NoMatch());

	shouldPopEdge.Connect(haveBusOwnership.Q());

	FullBundle bundle;
	bundle.Connect(0, addr);
	bundle.Connect(Naddr, data);
	circbuf.Connect(bundle, shouldPopEdge.Rise(), newWrite.Out());

	busIsFree.Connect(pSystemBus->OutCtrl().BusReq(), pSystemBus->OutCtrl().Ack());
	busIsFreeOrMine.Connect(haveBusOwnership.Q(), busIsFree.Out());
	wantTakeBus.Connect(busIsFree.Out(), circbuf.NonEmpty());
	wantReleaseBus.Connect(haveBusOwnership.Q(), pSystemBus->OutCtrl().Ack());
	haveBusOwnership.Connect(wantTakeBus.Out(), wantReleaseBus.Out());

	shouldOutputOnBus.Connect(circbuf.DidPop(), haveBusOwnership.Q());
	addrRequestBuf.Connect(circbuf.Out().Range<Naddr>(), shouldOutputOnBus.Out());
	dataRequestBuf.Connect(circbuf.Out().Range<N>(Naddr), shouldOutputOnBus.Out());
	busRequestBuf.Connect(Wire::ON, shouldOutputOnBus.Out());	
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusWriteBuffer<N, Naddr, Nbuf>::Update()
{
	lastMatchData.Update();
	lastMatchAddr.Update();
	lastWriteData.Update();
	lastWriteAddr.Update();
	newWrite.Update();

	shouldPopEdge.Update();
	circbuf.Update();

	{
		std::scoped_lock lk(pSystemBus->mBusMutex);
		busIsFree.Update();
		busIsFreeOrMine.Update();
		wantTakeBus.Update();
		wantReleaseBus.Update();
		haveBusOwnership.Update();
	}

	shouldOutputOnBus.Update();
	addrRequestBuf.Update();
	dataRequestBuf.Update();
	busRequestBuf.Update();

	//if (haveBusOwnership.Q().On())
	//{
	//	std::stringstream ss;
	//	ss << "Buffer tick: Have bus ownership." << std::endl;
	//	std::cout << ss.str();
	//}
	//if (busRequestBuf.Out().On())
	//{
	//	std::stringstream ss;
	//	ss << "----- buffer put " << dataRequestBuf.Out().Range<32>().Read() << " on bus" << std::endl;
	//	std::cout << ss.str();
	//}
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusWriteBuffer<N, Naddr, Nbuf>::ConnectToBus(SystemBus& bus)
{
	pSystemBus = &bus;
	pSystemBus->ConnectAddr(addrRequestBuf.Out());
	pSystemBus->ConnectData(dataRequestBuf.Out());
	pSystemBus->ConnectCtrl(shouldOutputOnBus.Out(), SystemBus::CtrlBit::Write);
	pSystemBus->ConnectCtrl(shouldOutputOnBus.Out(), SystemBus::CtrlBit::Req);
	pSystemBus->ConnectCtrl(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
}

template<unsigned int N, unsigned int Naddr, unsigned int Nbuf>
inline void BusWriteBuffer<N, Naddr, Nbuf>::DisconnectFromBus()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectAddr(addrRequestBuf.Out());
		pSystemBus->DisconnectData(dataRequestBuf.Out());
		pSystemBus->DisconnectCtrl(writeBusRequestBuf.Out(), SystemBus::CtrlBit::Write);
		pSystemBus->DisconnectCtrl(busRequestBuf.Out(), SystemBus::CtrlBit::Req);
		pSystemBus->DisconnectCtrl(haveBusOwnership.Q(), SystemBus::CtrlBit::BusReq);
	}
}