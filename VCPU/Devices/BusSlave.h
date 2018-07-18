#pragma once
#include "Bundle.h"
#include "Component.h"
#include "Inverter.h"
#include "AndGate.h"
#include "SystemBus.h"
#include "Register.h"
#include "FullAdder.h"

class BusSlaveBase : public Component
{
public:
	typedef Bundle<SystemBus::Naddr> AddrBundle;

	virtual ~BusSlaveBase();
	void Connect(SystemBus& bus, const BundleAny& data, const Wire& ack);
	void Update();
	void PostUpdate();

	const AddrBundle& GetAddr() { return pSystemBus->OutAddr(); }

	const Wire& Request() { return request.Out(); }
	const Wire& WriteRequest() { return writeRequest.Out(); }
	const Wire& ReadRequest() { return readRequest.Out(); }

protected:
	virtual void UpdateDataBuffer() = 0;

	SystemBus * pSystemBus;
	const BundleAny* mData;

	FullAdder mAckBuffer;
	AndGate mAckBuffer2;

	Inverter busAckInv;
	AndGate request;
	AndGate readRequest;
	AndGate writeRequest;
};

template <unsigned int N>
class BusSlaveConnector : public BusSlaveBase
{
public:
	typedef Bundle<N> DataBundle;
	using BusSlaveBase::BusSlaveBase;
	virtual ~BusSlaveConnector() {}

	void Connect(SystemBus& bus, const DataBundle& data, const Wire& ack);
	const DataBundle GetData() { return pSystemBus->OutData().Range<N>(); }
	void UpdateDataBuffer();

private:
	RegisterEnable<N> mDataBuffer;
};

template<unsigned int N>
inline void BusSlaveConnector<N>::Connect(SystemBus & bus, const DataBundle & data, const Wire & ack)
{
	BusSlaveBase::Connect(bus, mDataBuffer.Out(), ack);
	mDataBuffer.Connect(data, ack, mAckBuffer.Cout());
}

template<unsigned int N>
inline void BusSlaveConnector<N>::UpdateDataBuffer()
{
	mDataBuffer.Update();
}
