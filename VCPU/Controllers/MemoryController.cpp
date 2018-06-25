#include "MemoryController.h"
#include "DeviceController.h"


template <typename MemoryType>
MemoryController<MemoryType>::MemoryController(MemoryType* memory, DeviceController* receiver, DeviceController* transmitter)
	: pMemory(memory)
	, pReceiver(receiver)
	, pTransmitter(transmitter)
{
}

template <typename MemoryType>
void MemoryController<MemoryType>::Connect(const AddrBundle & addr, const DataBundle & data)
{
	isRegularMemoryAddr.Connect(isMemMappedIo.Out());
	memMappedMux.Connect({ pReceiver->GetControl(), pReceiver->GetData(), 
		pTransmitter->GetControl(), pTransmitter->GetData() }, addr.Range<2>(2));


}


template <typename MemoryType>
void MemoryController<MemoryType>::Update()
{
	isRegularMemoryAddr.Update();
	memMappedMux.Update();
}
