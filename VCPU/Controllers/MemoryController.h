#pragma once
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "NorGate.h"
#include "AndGate.h"
#include "MuxBundle.h"

// VCPU Memory map
// 0x00000000 -> 0x0fffffff		reserved
// 0x10000000 -> 0x1fffffff		user-level code
// 0x20000000 -> 0x7fffffff		user heap/stack
// 0x80000000 -> 0xffff0000		kernel reserved

// 0xffff0000			receiver control
// 0xffff0004			receiver data
// 0xffff0008			transmitter control
// 0xffff000c			transmitter data
// 0xffff0010			reserved

class DeviceController;

template <typename MemoryType>
class MemoryController : public Component
{
public:
	typedef Bundle<32> AddrBundle;
	typedef Bundle<32> DataBundle;
	MemoryController(MemoryType* memory, DeviceController* receiver, DeviceController* transmitter);
	void Connect(const AddrBundle& addr, const DataBundle& data);
	void Update();

private:
	DeviceController* pReceiver;
	DeviceController* pTransmitter;
	MemoryType* pMemory;

	NorGateN<8> bits8To15On;
	AndGateN<16> bitsHiOn;
	AndGate isMemMappedIo;
	Inverter isRegularMemoryAddr;
	MuxBundle<32, 4> memMappedMux;

};

