#include "DeviceController.h"
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

KeyboardController::~KeyboardController()
{
	StopUpdating();
	if (pSystemBus)
	{
		pSystemBus->DisconnectData(data.Out());
	}
}

void KeyboardController::Connect(SystemBus& bus)
{
	// 0xffff0000 is control, 0xffff0004 is data
	DeviceController::Connect(bus);
	pSystemBus->ConnectData(data.Out());

	incomingRequest.Connect({ &isMemMappedIo.Out(), &addrBit3Inv.Out(), &pSystemBus->OutCtrl().Req() });

	Bundle<32> dataBundle = Bundle<32>::OFF;
	dataBundle.Connect(0, c_in);
	data.Connect(dataBundle, Wire::ON, incomingDataRequest.Out());

	Bundle<32> controlBundle = Bundle<32>::OFF;
	controlBundle.Connect(0, pendingState.Q());
	control.Connect(controlBundle, Wire::ON, incomingControlRequest.Out());	

	pendingState.Connect(pending, incomingDataNow.Rise());
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