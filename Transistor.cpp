#include "Transistor.h"

void Transistor::Connect(const Wire& b, const Wire& c)
{
	base = &b;
	collector = &c;
	Update();
}

void Transistor::Update()
{
	emitter.Set(base && collector);
}
