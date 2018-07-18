#include "DeviceController.h"
#include "SystemBus.h"
#include <conio.h>
#include <sstream>


DeviceController::~DeviceController()
{
	StopUpdating();
}

void DeviceController::Connect(SystemBus & bus)
{
	busConnector.Connect(bus, outData.Out(), outServicedRequest.Out());
	
	bits8To15On.Connect(busConnector.GetAddr().Range<8>(8));
	bitsHiOn.Connect(busConnector.GetAddr().Range<16>(16));
	isMemMappedIo.Connect(bits8To15On.Out(), bitsHiOn.Out());	
	addrBit2Inv.Connect(busConnector.GetAddr()[2]);
	addrBit3Inv.Connect(busConnector.GetAddr()[3]);

	incomingRequest.Connect(myAddress.Out(), busConnector.Request());
	dataRequest.Connect(incomingRequest.Out(), busConnector.GetAddr()[2]);
	controlRequest.Connect(incomingRequest.Out(), addrBit2Inv.Out());

	outServicedRequest.Connect(incomingRequest.Out(), Wire::ON);
	outData.Connect(data.Out(), control.Out());
}

void DeviceController::Update()
{
	busConnector.Update();

	bits8To15On.Update();
	bitsHiOn.Update();
	isMemMappedIo.Update();
	addrBit2Inv.Update(); 
	addrBit3Inv.Update();

	myAddress.Update();
	incomingRequest.Update();
	dataRequest.Update();
	controlRequest.Update();

	InternalUpdate();

	outServicedRequest.Update();
	outData.Update();

	busConnector.PostUpdate();
}
