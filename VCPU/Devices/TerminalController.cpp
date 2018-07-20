#include "TerminalController.h"
#include "SystemBus.h"
#include <iostream>


void TerminalController::Connect(SystemBus& bus)
{
	// 0xffff0008 is control, 0xffff000c is data
	DeviceController::Connect(bus);
	myAddress.Connect(isMemMappedIo.Out(), busConnector.GetAddr()[3]);

	Bundle<32> controlBundle = Bundle<32>::OFF;
	controlBundle.Connect(0, pendingState.Q());
	control.Connect(controlBundle, Wire::ON, controlRequest.Out());

	data.Connect(busConnector.GetData().Range<32>(), dataRequest.Out(), Wire::OFF);

	pendingState.Connect(pendingState.NotQ(), dataRequest.Out());
	pendingState.Update();
}

void TerminalController::InternalUpdate()
{
 	data.Update();
	pendingState.Update();

	if (pendingState.NotQ().On())
	{
		//std::cout << (unsigned char)data.ReadReg().Range<8>().UnsignedRead();
	}
	control.Update();
}