#include "AndGate.h"


void AndGate::Connect(const Wire& a, const Wire& b)
{
	in1 = &a;
	in2 = &b;
}

