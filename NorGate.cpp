#include "NorGate.h"


void NorGate::Connect(const Wire& a, const Wire& b)
{
	or.Connect(a, b);
	inv.Connect(or.Out());
}

void NorGate::Update()
{
	or.Update();
	inv.Update();
}
