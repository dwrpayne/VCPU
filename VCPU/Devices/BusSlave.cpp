#include "BusSlave.h"


BusSlaveBase::~BusSlaveBase()
{
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(*mData);
		pSystemBus->DisconnectCtrl(mAckBuffer.Cout(), SystemBus::CtrlBit::Ack);
	}
}

void BusSlaveBase::Connect(SystemBus& bus, const BundleAny& data, const Wire & ack)
{
	pSystemBus = &bus;
	mData = &data;
	pSystemBus->ConnectData(data);
	pSystemBus->ConnectCtrl(mAckBuffer2.Out(), SystemBus::CtrlBit::Ack);

	busAckInv.Connect(pSystemBus->OutCtrl().Ack());
	request.Connect(pSystemBus->OutCtrl().Req(), busAckInv.Out());
	readRequest.Connect(request.Out(), pSystemBus->OutCtrl().Read());
	writeRequest.Connect(request.Out(), pSystemBus->OutCtrl().Write());

	mAckBuffer.Connect(pSystemBus->OutCtrl().Req(), ack, mAckBuffer.Cout());
	mAckBuffer2.Connect(pSystemBus->OutCtrl().Req(), mAckBuffer.Cout());
}

void BusSlaveBase::Update()
{
	busAckInv.Update();
	request.Update();
	readRequest.Update();
	writeRequest.Update();
}

void BusSlaveBase::PostUpdate()
{
	mAckBuffer.Update();
	UpdateDataToBusBuffer();
	mAckBuffer2.Update();
}
