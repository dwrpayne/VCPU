#include "NorGate.h"

NorGate::NorGate(const Wire& a, const Wire& b)
	: or(a, b)
	, inv(or.Out())
{
	Update();
}

void NorGate::Connect(const Wire& a, const Wire& b)
{
	or.Connect(a, b);
	inv.Connect(or.Out());
	Update();
}

void NorGate::Update()
{
	or.Update();
	inv.Update();
}
