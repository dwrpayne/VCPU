#include "FullAdder.h"

void FullAdder::Connect(const Wire & a, const Wire & b, const Wire & c)
{
	xorIn.Connect(a, b);
	andIn.Connect(a, b);
	xorOut.Connect(xorIn.Out(), c);
	andOut.Connect(xorIn.Out(), c);
	orGate.Connect(andIn.Out(), andOut.Out());
}

void FullAdder::Update()
{
	xorIn.Update();
	andIn.Update();
	xorOut.Update();
	andOut.Update();
	orGate.Update();
}
