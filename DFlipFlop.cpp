#include "DFlipFlop.h"

DFlipFlop::DFlipFlop(const Wire& d, const Wire& e)
	: invD(d)
	, nandD(d, e)
	, nandDinv(invD.Out(), e)
	, nandR(nandD.Out(), nandS.Out())
	, nandS(nandDinv.Out(), nandR.Out())

{
}

void DFlipFlop::Update()
{

}
