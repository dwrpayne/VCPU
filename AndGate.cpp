#include "AndGate.h"

AndGate::AndGate()
	: in1(&WIRE_DISCONNECTED)
	, in2(&WIRE_DISCONNECTED)
	, Out(out)
{}

AndGate::AndGate(const PinIn& a, const PinIn& b)
	: in1(&a)
	, in2(&b)
	, Out(out)
{
	Update();
}

void AndGate::Connect(const PinIn& a, const PinIn& b)
{
	in1 = &a;
	in2 = &b;
	Update();
}

void AndGate::Update()
{
	out.Set(in1->On() && in2->On());
}
