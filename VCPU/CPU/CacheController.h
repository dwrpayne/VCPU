#pragma once
#include "Component.h"
#include "Bundle.h"
#include "Register.h"
#include "Decoder.h"
#include "MultiGate.h"
#include "Masker.h"
#include "MuxBundle.h"
#include "CacheLine.h"
#include "Memory.h"
#include "ByteMask.h"
#include "SystemBus.h"
#include "TriStateBuffer.h"

class CacheController : public Component
{
public:
	void Connect(const Wire& read, const Wire& write, const Wire& busack, const Wire& uncacheableaddr, const Wire& cachehit, const Wire& cachedirty);
	void Update();
	
	
private:


	JKFlipFlop idle;
	JKFlipFlop waitingForMemoryRead;
	JKFlipFlop waitingForMemoryWrite;
	JKFlipFlop waitingForUncachedAck;

	// State transitions
	AndGate moveToIdle;

	
	OrGate readOrWrite;
	AndGate uncachedWrite;
	AndGate uncachedRead;
	OrGate uncachedReadOrWrite;
	Inverter cacheableAddr;
	AndGate cachedWrite;
	AndGate cachedRead;
	OrGate cachedReadOrWrite;
	AndGate gotResponseFromMemory;
	AndGate gotReadDataFromMemory;
	Inverter gotResponseFromMemoryInv;
	AndGate waitingForUncachedData;

	AndGateN<3> evictedDirty;
	Inverter notEvictedDirty;
	Inverter cacheMissInternal;
	AndGate cacheMiss;
	AndGate cacheableAddrCacheMiss;
	AndGate writeBufferFull;
	OrGateN<3> needStall;
	Inverter needStallInv;
	AndGateN<3> cachedReadReqOkay;
	OrGate shouldSendReadReq;
	OrGate shouldSendWriteReq;


	AndGate canHaveBusOwnership;
	NorGate busIsFree;
	OrGate busIsFreeOrMine;
	DFlipFlop haveBusOwnership;
	AndGate shouldOutputOnBus;
	AndGate shouldOutputDataBus;

};
