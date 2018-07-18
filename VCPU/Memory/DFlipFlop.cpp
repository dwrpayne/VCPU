#include "DFlipFlop.h"

void DFlipFlop::Connect(const Wire& d, const Wire& e)
{
	invD.Connect(d);
	andD.Connect(d, e);
	andDinv.Connect(invD.Out(), e);
	norDinv.Connect(andDinv.Out(), norD.Out());
	norD.Connect(andD.Out(), norDinv.Out());
}

