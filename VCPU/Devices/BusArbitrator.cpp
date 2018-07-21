#include "BusArbitrator.h"

BusArbitrator::~BusArbitrator()
{
	for (int i = 0; i < N; i++)
	{
		pSystemBus->DisconnectCtrl(outGrants[i].Out(), SystemBus::GetBusGrant(i));
	}
}

void BusArbitrator::Connect(SystemBus& bus)
{
	pSystemBus = &bus;

	busFree.Connect(pSystemBus->OutCtrl().Ack(), pSystemBus->OutCtrl().Req());
	for (int i = 0; i < N; i++)
	{
		reqs[i].Connect(pSystemBus->OutCtrl().BusReq(i), Wire::ON);
		nextGrants[i].Connect(reqs[i].Out(), i == 0 ? Wire::ON : nextGrantInvs[i-1].Out());
		nextGrantInvs[i].Connect(nextGrants[i].Out());
		outGrants[i].Connect(nextGrants[i].Out(), busFree.Out());
		pSystemBus->ConnectCtrl(outGrants[i].Out(), SystemBus::GetBusGrant(i));

	}
}

void BusArbitrator::Update()
{
	busFree.Update();
	if (pSystemBus->OutCtrl().BusReq(0).On() && pSystemBus->OutCtrl().BusReq(1).On())
	{
		//__debugbreak();
	}
	for (int i = 0; i < N; i++)
	{
		reqs[i].Update();
		nextGrants[i].Update();
		nextGrantInvs[i].Update();
		outGrants[i].Update();
	}
}

