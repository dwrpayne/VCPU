#include "TriStateBuffer.h"

void TriState::Connect(const Wire & in, const Wire & e)
{
	input = &in;
	enable = &e;
}

void TriState::Update()
{
	out.Set(input->On() && enable->On());
	out.SetHiZ(!enable->On());
}
