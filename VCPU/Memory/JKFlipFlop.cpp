#include "JKFlipFlop.h"


void JKFlipFlop::Connect(const Wire& j, const Wire& k)
{
	andj.Connect(j, latch.NotQ());
	andk.Connect(latch.Q(), k);
	latch.Connect(andj.Out(), andk.Out());
}

void JKFlipFlop::Update()
{
	andj.Update();
	andk.Update();
	latch.Update();
}

