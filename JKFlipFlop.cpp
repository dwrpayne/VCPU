#include "JKFlipFlop.h"

JKFlipFlop::JKFlipFlop(const Wire& j, const Wire& k)
	: andj(j, latch.NotQ())
	, andk(latch.Q(), k)
	, latch(andj.Out, andk.Out)
{
}

void JKFlipFlop::Connect(const Wire& j, const Wire& k)
{
	andj.Connect(j, latch.NotQ());
}

void JKFlipFlop::Update()
{
	andj.Update();
	andk.Update();
	latch.Update();
}
