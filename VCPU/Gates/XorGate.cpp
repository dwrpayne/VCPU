#include "XorGate.h"


void XorGate::Connect(const Wire& a, const Wire& b)
{
	nand.Connect(a, b);
	or.Connect(a, b);
	and.Connect(nand.Out(), or.Out());
}

void XorGate::Update()
{
	nand.Update();
	or.Update();
	and.Update();
}
