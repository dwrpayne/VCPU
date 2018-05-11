#include "NandGate.h"


void NandGate::Connect(const Wire& a, const Wire& b)
{
	and.Connect(a, b);
	inv.Connect(and.Out());
}

void NandGate::Update()
{
	and.Update();
	inv.Update();
}
