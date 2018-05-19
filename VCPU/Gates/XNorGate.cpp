#include "XNorGate.h"


void XNorGate::Connect(const Wire& a, const Wire& b)
{
	and.Connect(a, b);
	aInv.Connect(a);
	bInv.Connect(b);
	invAnd.Connect(aInv.Out(), bInv.Out());
	orGate.Connect(and.Out(), invAnd.Out());
}

void XNorGate::Update()
{
	and.Update();
	aInv.Update();
	bInv.Update();
	invAnd.Update();
	orGate.Update();
}