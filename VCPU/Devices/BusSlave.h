#pragma once
#include "Bundle.h"
#include "Component.h"
#include "Inverter.h"
#include "AndGate.h"
#include "SystemBus.h"
#include "Register.h"
#include "FullAdder.h"


class BusSlaveConnector : public Component
{
public:
	static const int N = SystemBus::Ndata;
	typedef Bundle<N> DataBundle;
	typedef Bundle<SystemBus::Naddr> AddrBundle;

	~BusSlaveConnector();
	void Connect(SystemBus& bus, const DataBundle& data, const Wire& ack);
	void Update();
	void PostUpdate();

	const AddrBundle& GetAddr() { return pSystemBus->OutAddr(); }
	const DataBundle& GetData() { return pSystemBus->OutData(); }

	const Wire& Request() { return request.Out(); }
	const Wire& WriteRequest() { return writeRequest.Out(); }
	const Wire& ReadRequest() { return readRequest.Out(); }

private:
	SystemBus* pSystemBus;

	RegisterEnable<SystemBus::Ndata> mDataBuffer;
	FullAdder mAckBuffer;
	AndGate mAckBuffer2;

	Inverter busAckInv;
	AndGate request;
	AndGate readRequest;
	AndGate writeRequest;
};

