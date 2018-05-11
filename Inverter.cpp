#include "Inverter.h"

void Inverter::Connect(const Wire& a)
{
	in = &a;
	Update();
}

void Inverter::Update()
{
	out.Set(!in->On());
}
