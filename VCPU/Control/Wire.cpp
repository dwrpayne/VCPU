#include "Wire.h"

static const Wire WIRE_OFF(false);
static const Wire WIRE_ON(true);
int Wire::id_counter = 0;

std::ostream& operator<<(std::ostream& os, const Wire& w)
{
	os << w.On();
	return os;
}

inline Wire::Wire(bool initial_state)
	: state(initial_state)
	, id(id_counter++)
{ 
}
