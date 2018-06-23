#pragma once

#include "Component.h"
#include "SystemBus.h"
#include "Register.h"
#include "Decoder.h"

// VCPU Memory map
// 0x00000000 -> 0x0fffffff		user-level code
// 0x10000000 -> 0x7fffffff		user heap/stack
// 0x80000000 -> 0xffff0000		kernel reserved

// 0xffff0000					receiver control
// 0xffff0004					receiver data
// 0xffff0008					transmitter control
// 0xffff000c					transmitter data
// 0xffff0010 -> 0xffffffff		reserved

class SystemBusBuffer : public Component
{
public:
	static const int Nctrl = SystemBus::Nctrl;
	static const int Naddr = SystemBus::Naddr;
	static const int Ndata = SystemBus::Ndata;

	void Connect(SystemBus& bus);
	void Update();

	const Bundle<SystemBus::Ndata>& OutData() const { return data.Out(); }
	const Bundle<SystemBus::Naddr>& OutAddr() const { return addr.Out(); }
	const SystemBus::ControlBundle& OutCtrl() const { return static_cast<const SystemBus::ControlBundle&>(ctrl.Out()); }

	const Wire& UserCode() const { return loReserved.Out(); }
	const Wire& UserData() const { return usermem.Out(); }
	const Wire& ReceiverControl() const { return memmapIoDec.Out()[0]; }
	const Wire& ReceiverData() const { return memmapIoDec.Out()[1]; }
	const Wire& TransmitterControl() const { return memmapIoDec.Out()[2]; }
	const Wire& TransmitterData() const { return memmapIoDec.Out()[3]; }

	SystemBus* GetSystemBus() { return pSystemBus; }

private:
	SystemBus * pSystemBus;

	NorGateN<4> loReserved;
	NorGate usermem;
	AndGateN<16> memmapIo;
	Decoder<4> memmapIoDec;
	NorGateN<3> kernelmem;

	Register<SystemBus::Ndata> data;
	Register<SystemBus::Naddr> addr;
	Register<SystemBus::Nctrl> ctrl;
};


inline void SystemBusBuffer::Connect(SystemBus& bus)
{
	pSystemBus = &bus;

	loReserved.Connect(addr.Out().Range<4>(Naddr - 4));
	usermem.Connect(loReserved.Out(), addr.Out()[Naddr - 1]);
	memmapIo.Connect(addr.Out().Range<16>(Naddr - 16));
	memmapIoDec.Connect(addr.Out().Range<2>(2), memmapIo.Out());
	kernelmem.Connect({ &loReserved.Out(), &usermem.Out(), &memmapIo.Out() });

	data.Connect(bus.OutData(), Wire::ON);
	addr.Connect(bus.OutAddr(), Wire::ON);
	ctrl.Connect(bus.OutCtrl(), Wire::ON);
}

inline void SystemBusBuffer::Update()
{
	loReserved.Update();
	usermem.Update();
	memmapIo.Update();
	memmapIoDec.Update();
	kernelmem.Update();

	data.Update();
	addr.Update();
	ctrl.Update();
}
