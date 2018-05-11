#include "OrGate.h"

void OrGate::Connect(const Wire& a, const Wire& b)
{
	in1 = &a;
	in2 = &b;
	Update();
}

void OrGate::Update()
{
	out.Set(in1->On() || in2->On());
}
