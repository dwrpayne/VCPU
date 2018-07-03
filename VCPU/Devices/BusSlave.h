#pragma once
#include "Bundle.h"
#include "Component.h"
#include "Inverter.h"
#include "AndGate.h"
#include "SystemBus.h"


class BusSlaveConnector : public Component
{
public:
	typedef Bundle<SystemBus::Ndata> DataBundle;
	typedef Bundle<SystemBus::Naddr> AddrBundle;

	~BusSlaveConnector();
	void Connect(SystemBus& bus, const BundleAny& data, const Wire& ack);
	void Update();

	const AddrBundle& GetAddr() { return pSystemBus->OutAddr(); }
	const DataBundle& GetData() { return pSystemBus->OutData(); }

	const Wire& Request() { return request.Out(); }
	const Wire& WriteRequest() { return writeRequest.Out(); }
	const Wire& ReadRequest() { return readRequest.Out(); }

private:
	const Wire* mAck;
	const BundleAny* mData;
	SystemBus* pSystemBus;

	Inverter busAckInv;
	AndGate request;
	AndGate readRequest;
	AndGate writeRequest;
};

