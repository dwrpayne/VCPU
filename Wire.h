#pragma once
#include <iostream>

class Wire
{
public:
	Wire() { state = false; }
	Wire(bool initial_state) { state = initial_state; }

	void Set(bool new_state) { state = new_state; }
	
	bool On() const { return state; }
	Wire& operator=(const Wire& other) { state = other.state; }
	
private:
	bool state;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);

extern const Wire WIRE_ON;
extern const Wire WIRE_OFF;
extern const Wire WIRE_DISCONNECTED;