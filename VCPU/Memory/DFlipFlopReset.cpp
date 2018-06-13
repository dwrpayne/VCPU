#include "DFlipFlopReset.h"

void DFlipFlopReset::Connect(const Wire& d, const Wire& e, const Wire& reset)
{
	invD.Connect(d);
	invR.Connect(reset);
	nandD.Connect(d, e);
	nandDinv.Connect(invD.Out(), e);
	nandR.Connect(nandD.Out(), nandS.Out());
	nandS.Connect({&nandDinv.Out(), &nandR.Out(), &invR.Out()});
}

void DFlipFlopReset::Update()
{
	invD.Update();
	invR.Update();
	nandD.Update();
	nandDinv.Update();
	nandS.Update();
	nandR.Update();
	nandS.Update();
}
