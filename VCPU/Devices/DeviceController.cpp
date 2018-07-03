#include "DeviceController.h"
#include "SystemBus.h"
#include <conio.h>
#include <sstream>


DeviceController::~DeviceController()
{
	StopUpdating();
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(control.Out());
		pSystemBus->DisconnectCtrl(outServicedRequest.Out(), SystemBus::CtrlBit::Ack);
	}
}

void DeviceController::Connect(SystemBus & bus)
{
	pSystemBus = &bus;
	pSystemBus->ConnectData(control.Out());
	pSystemBus->ConnectCtrl(outServicedRequest.Out(), SystemBus::CtrlBit::Ack);
	
	bits8To15On.Connect(pSystemBus->OutAddr().Range<8>(8));
	bitsHiOn.Connect(pSystemBus->OutAddr().Range<16>(16));
	isMemMappedIo.Connect(bits8To15On.Out(), bitsHiOn.Out());
	
	addrBit2Inv.Connect(pSystemBus->OutAddr()[2]);
	addrBit3Inv.Connect(pSystemBus->OutAddr()[3]);
	incomingDataRequest.Connect(incomingRequest.Out(), pSystemBus->OutAddr()[2]);
	incomingControlRequest.Connect(incomingRequest.Out(), addrBit2Inv.Out());
	incomingWriteRequest.Connect(incomingRequest.Out(), pSystemBus->OutCtrl().Write());
	incomingDataNow.Connect(incomingDataRequest.Out());

	outServicedRequest.Connect(incomingRequest.Out(), Wire::ON);
}

void DeviceController::Update()
{
	bits8To15On.Update();
	bitsHiOn.Update();
	isMemMappedIo.Update();
	addrBit2Inv.Update(); 
	addrBit3Inv.Update();

	incomingRequest.Update();
	incomingDataRequest.Update();
	incomingControlRequest.Update();
	incomingWriteRequest.Update();
	incomingDataNow.Update();

	InternalUpdate();

	outServicedRequest.Update();
}
