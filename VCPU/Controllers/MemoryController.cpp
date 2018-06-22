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
	bits8To15On.Connect(addr.Range<8>(8));
	bitsHiOn.Connect(addr.Range<16>(16));
	isMemMappedIo.Connect(bits8To15On.Out(), bitsHiOn.Out());
	isRegularMemoryAddr.Connect(isMemMappedIo.Out());
	memMappedMux.Connect({ pReceiver->GetControl(), pReceiver->GetData(), 
		pTransmitter->GetControl(), pTransmitter->GetData() }, addr.Range<2>(2));


}


template <typename MemoryType>
void MemoryController<MemoryType>::Update()
{
	bits8To15On.Update();
	bitsHiOn.Update();
	isMemMappedIo.Update();
	isRegularMemoryAddr.Update();
	memMappedMux.Update();
}
