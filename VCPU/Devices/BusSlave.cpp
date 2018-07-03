#include "BusSlave.h"
#include "SystemBus.h"


BusSlaveConnector::~BusSlaveConnector()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(*mData);
		pSystemBus->DisconnectCtrl(*mAck, SystemBus::CtrlBit::Ack);
	}
}

void BusSlaveConnector::Connect(SystemBus & bus, const BundleAny& data, const Wire & ack)
{
	mAck = &ack;
	mData = &data;
	pSystemBus = &bus;
	pSystemBus->ConnectData(data);
	pSystemBus->ConnectCtrl(ack, SystemBus::CtrlBit::Ack);

	busAckInv.Connect(pSystemBus->OutCtrl().Ack());
	request.Connect(pSystemBus->OutCtrl().Req(), busAckInv.Out());
	readRequest.Connect(request.Out(), pSystemBus->OutCtrl().Read());
	writeRequest.Connect(request.Out(), pSystemBus->OutCtrl().Write());
}

void BusSlaveConnector::Update()
{
	busAckInv.Update();
	request.Update();
	readRequest.Update();
	writeRequest.Update();
}
