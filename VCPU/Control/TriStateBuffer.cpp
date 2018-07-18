#include "TriStateBuffer.h"

void TriState::Connect(const Wire & in, const Wire & e)
{
	input = &in;
	enable = &e;
}
