#include "Inverter.h"

void Inverter::Connect(const Wire& a)
{
	in = &a;
}

void Inverter::Update()
{
	out.Set(!in->On());
}
