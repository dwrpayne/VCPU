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
	typedef Bundle<32> DataBundle;
	using ThreadedAsyncComponent::ThreadedAsyncComponent;

	virtual ~DeviceController();
	virtual void Connect(SystemBus& bus);

	const DataBundle& GetControl() { return control.Out(); }
	const DataBundle& GetData() { return data.Out(); }

protected:
	virtual void Update();
	virtual void InternalUpdate() = 0;

	AndGateN<3> incomingRequest;
	RegisterEnable<32> control;
	RegisterEnable<32> data;

	SystemBus* pSystemBus;
	NorGateN<8> bits8To15On;
	AndGateN<16> bitsHiOn;
	AndGate isMemMappedIo;
	Inverter addrBit2Inv;
	Inverter addrBit3Inv;
	AndGate incomingControlRequest;
	AndGate incomingDataRequest;
	AndGate incomingWriteRequest;
	EdgeDetector incomingDataNow;

	AndGate outServicedRequest;
	MultiGate<OrGate, 32> outData;

	Wire pending;
	JKFlipFlop pendingState;
};
