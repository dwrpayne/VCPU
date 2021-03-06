#include "KeyboardController.h"
#include "SystemBus.h"
#include <conio.h>


void KeyboardController::Connect(SystemBus& bus)
{
	// 0xffff0000 is control, 0xffff0004 is data
	DeviceController::Connect(bus);

	myAddress.Connect(isMemMappedIo.Out(), addrBit3Inv.Out());

	Bundle<32> dataBundle = Bundle<32>::OFF;
	dataBundle.Connect(0, c_in);
	data.Connect(dataBundle, Wire::ON, dataRequest.Out());

	Bundle<32> controlBundle = Bundle<32>::OFF;
	controlBundle.Connect(0, pendingState.Q());
	control.Connect(controlBundle, Wire::ON, controlRequest.Out());	

	pendingState.Connect(pending, dataRequest.Out());
}

void KeyboardController::InternalUpdate()
{
	if (_kbhit())
	{
		if (int c = _getch())
		{
			c_in.Write(c);
			pending.Set(true);
		}
	}

	pendingState.Update();
	pending.Set(false);

	control.Update();
	data.Update();
}
