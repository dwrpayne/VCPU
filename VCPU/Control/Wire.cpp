#include "Wire.h"

unsigned int Wire::id_counter = 0;
const Wire Wire::OFF(false);
const Wire Wire::ON(true);

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
