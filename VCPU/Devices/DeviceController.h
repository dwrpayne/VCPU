#pragma once
#include "Bundle.h"
#include "ThreadedComponent.h"
#include "Register.h"
#include "JKFlipFlop.h"
#include "EdgeDetector.h"
#include "BusSlave.h"
#include "MultiGate.h"
#include "OrGate.h"
#include "AndGate.h"
#include "NorGate.h"

class SystemBus;

class DeviceController : public ThreadedAsyncComponent
{
public:
	static const unsigned int N = 32;
	typedef Bundle<32> DataBundle;
	using ThreadedAsyncComponent::ThreadedAsyncComponent;

	virtual ~DeviceController();
	virtual void Connect(SystemBus& bus);

	const DataBundle& GetControl() { return control.Out(); }
	const DataBundle& GetData() { return data.Out(); }

protected:
	virtual void Update();
	virtual void InternalUpdate() = 0;

	BusSlaveConnector<N> busConnector;

	AndGate myAddress;
	RegisterEnable<N> control;
	RegisterEnable<N> data;

	NorGateN<8> bits8To15On;
	AndGateN<16> bitsHiOn;
	AndGate isMemMappedIo;
	Inverter addrBit2Inv;
	Inverter addrBit3Inv;
	AndGate incomingRequest;
	AndGate controlRequest;
	AndGate dataRequest;

	AndGate outServicedRequest;
	MultiGate<OrGate, N> outData;

	Wire pending;
	JKFlipFlop pendingState;
};
