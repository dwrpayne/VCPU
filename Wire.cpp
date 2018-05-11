#include "Wire.h"

static const Wire WIRE_ON(true);
static const Wire WIRE_OFF(false);
static const Wire WIRE_DISCONNECTED(false);


std::ostream& operator<<(std::ostream& os, const Wire& dt)
{
	os << dt.On();
	return os;
}