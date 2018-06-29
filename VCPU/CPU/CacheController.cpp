#include "CacheController.h"

void CacheController::Connect(const Wire& read, const Wire& write, const Wire& busack, const Wire& uncacheableaddr, const Wire& cachehit, const Wire& cachedirty)
{
	//readOrWrite.Connect(read, write);
	//uncachedWrite.Connect(uncacheableaddr, write);
	//uncachedRead.Connect(uncacheableaddr, read);
	//uncachedReadOrWrite.Connect(uncacheableaddr, readOrWrite.Out());
	//cacheableAddr.Connect(uncacheableaddr);
	//cachedWrite.Connect(cacheableAddr.Out(), write);
	//cachedRead.Connect(cacheableAddr.Out(), read);
	//cachedReadOrWrite.Connect(cacheableAddr.Out(), readOrWrite.Out());

	//gotResponseFromMemory.Connect(haveBusOwnership.Q(), busack);
	//gotCacheableDataFromMemory.Connect(gotResponseFromMemory.Out(), waitingForCacheRead.Q());
	//gotResponseFromMemoryInv.Connect(gotResponseFromMemory.Out());
	//waitingForUncachedData.Connect(gotResponseFromMemoryInv.Out(), uncachedReadOrWrite.Out());

	//cacheMissInternal.Connect(cachehit);
	//cacheMiss.Connect(cacheMissInternal.Out(), readOrWrite.Out());
	//cacheableAddrCacheMiss.Connect(cacheMissInternal.Out(), cachedReadOrWrite.Out());

	//idle.Connect()


	//evictedDirty.Connect({ &cacheableAddrCacheMiss.Out(), &cachedirty, &gotResponseFromMemoryInv.Out() });
	//notEvictedDirty.Connect(evictedDirty.Out());
	//writeBufferFull.Connect(evictedDirty.Out(), Wire::ON);
	//needStall.Connect({ &cacheableAddrCacheMiss.Out(), &writeBufferFull.Out(), &waitingForUncachedData.Out() });
	//needStallInv.Connect(needStall.Out());
	//cachedReadReqOkay.Connect({ &cacheMiss.Out(), &notEvictedDirty.Out(), &cacheableAddr.Out() });

	//shouldSendReadReq.Connect(cachedReadReqOkay.Out(), uncachedRead.Out());
	//shouldSendWriteReq.Connect(evictedDirty.Out(), uncachedWrite.Out());

	//canHaveBusOwnership.Connect(needStall.Out(), gotResponseFromMemoryInv.Out());

	//busIsFree.Connect(pSystemBus->OutCtrl().BusReq(), pSystemBus->OutCtrl().Ack());
	//busIsFreeOrMine.Connect(haveBusOwnership.Q(), busIsFree.Out());
	//haveBusOwnership.Connect(canHaveBusOwnership.Out(), busIsFreeOrMine.Out());

	//shouldOutputOnBus.Connect(haveBusOwnership.Q(), Wire::ON);
	//shouldOutputDataBus.Connect(shouldOutputOnBus.Out(), shouldSendWriteReq.Out());
}

void CacheController::Update()
{
}
