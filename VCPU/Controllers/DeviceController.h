#pragma once
#include "Bundle.h"
#include "Tools/MagicBundle.h"
#include "ThreadedComponent.h"
#include "CPU/SystemBus.h"
#include "Register.h"
#include "JKFlipFlop.h"
#include "EdgeDetector.h"

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

	SystemBus * pSystemBus;
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

	Wire pending;
	JKFlipFlop pendingState;
};

class KeyboardController : public DeviceController
{
public:
	KeyboardController()
		: DeviceController(L"Keyboard Controller Thread")
	{}
	virtual ~KeyboardController();
	virtual void Connect(SystemBus& bus);
	virtual void InternalUpdate();
	
private:
	MagicBundle<8> c_in;
};


class TerminalController : public DeviceController
{
public:
	TerminalController()
		: DeviceController(L"Terminal Controller Thread")
	{}
	virtual ~TerminalController() 
	{
		StopUpdating();
	}
	virtual void Connect(SystemBus& bus);
	virtual void InternalUpdate();
};

