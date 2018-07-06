#include "BusSlave.h"
#include "SystemBus.h"


BusSlaveConnector::~BusSlaveConnector()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(mDataBuffer.Out());
		pSystemBus->DisconnectCtrl(mAckBuffer.Cout(), SystemBus::CtrlBit::Ack);
	}
}

void BusSlaveConnector::Connect(SystemBus & bus, const DataBundle& data, const Wire & ack)
{
	pSystemBus = &bus;
	pSystemBus->ConnectData(mDataBuffer.Out());
	pSystemBus->ConnectCtrl(mAckBuffer.Cout(), SystemBus::CtrlBit::Ack);

	busAckInv.Connect(pSystemBus->OutCtrl().Ack());
	request.Connect(pSystemBus->OutCtrl().Req(), busAckInv.Out());
	readRequest.Connect(request.Out(), pSystemBus->OutCtrl().Read());
	writeRequest.Connect(request.Out(), pSystemBus->OutCtrl().Write());

	mAckBuffer.Connect(request.Out(), ack, mAckBuffer.Cout());

	mDataBuffer.Connect(data, ack, mAckBuffer.Cout());
}

void BusSlaveConnector::Update()
{
	busAckInv.Update();
	request.Update();
	readRequest.Update();
	writeRequest.Update();
	mAckBuffer.Update();
	mDataBuffer.Update();
}
