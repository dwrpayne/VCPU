#pragma once
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "NorGate.h"
#include "AndGate.h"
#include "MuxBundle.h"

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

	Inverter isRegularMemoryAddr;
	MuxBundle<32, 4> memMappedMux;

};

