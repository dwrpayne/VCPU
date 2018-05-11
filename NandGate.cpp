#include "NandGate.h"

NandGate::NandGate(const Wire& a, const Wire& b)
	: and(a, b)
	, inv(and.Out)
{
	Update();
}

void NandGate::Connect(const Wire& a, const Wire& b)
{
	Update();
}

void NandGate::Update()
{
	and.Update();
	inv.Update();
}
