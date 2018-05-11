#pragma once
#include <iostream>

#ifdef DEBUG
#define CHOOSE_WIRE(b) (b ? WIRE_ON : WIRE_OFF)
#endif

class Wire
{
public:
	Wire(bool initial_state = false);

	void Set(bool new_state) { state = new_state; }
	
	bool On() const { return state; }

	friend std::ostream& operator<<(std::ostream& os, const Wire& dt);
	
private:
	static int id_counter;
	int id;
	bool state;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);

extern const Wire WIRE_ON;
extern const Wire WIRE_OFF;