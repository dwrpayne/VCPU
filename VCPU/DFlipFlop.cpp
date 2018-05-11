#include "DFlipFlop.h"

void DFlipFlop::Connect(const Wire& d, const Wire& e)
{
	invD.Connect(d);
	nandD.Connect(d, e);
	nandDinv.Connect(invD.Out(), e);
	nandR.Connect(nandD.Out(), nandS.Out());
	nandS.Connect(nandDinv.Out(), nandR.Out());
}

void DFlipFlop::Update()
{
	invD.Update();
	nandD.Update();
	nandDinv.Update();
	nandR.Update();
	nandS.Update();
	nandR.Update();
}
