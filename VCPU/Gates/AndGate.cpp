#include "AndGate.h"


void AndGate::Connect(const Wire& a, const Wire& b)
{
	in1 = &a;
	in2 = &b;
}

void AndGate::Update()
{
	out.Set(in1->On() && in2->On());
}
