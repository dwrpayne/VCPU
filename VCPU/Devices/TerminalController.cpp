#include "TerminalController.h"
#include "SystemBus.h"
#include <iostream>


void TerminalController::Connect(SystemBus& bus)
{
	// 0xffff0008 is control, 0xffff000c is data
	DeviceController::Connect(bus);
	incomingRequest.Connect({ &isMemMappedIo.Out(), &pSystemBus->OutAddr()[3], &pSystemBus->OutCtrl().Req() });

	Bundle<32> controlBundle = Bundle<32>::OFF;
	controlBundle.Connect(0, pendingState.Q());
	control.Connect(controlBundle, Wire::ON, incomingControlRequest.Out());

	data.Connect(pSystemBus->OutData().Range<32>(), incomingDataRequest.Out(), Wire::OFF);

	pendingState.Connect(pendingState.NotQ(), incomingDataNow.Rise());
	pendingState.Update();
}

void TerminalController::InternalUpdate()
{
 	data.Update();
	pendingState.Update();

	if (pendingState.NotQ().On())
	{
		std::cout << (unsigned char)data.ReadReg().Range<8>().UnsignedRead();
	}
	control.Update();
}