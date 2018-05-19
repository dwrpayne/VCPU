#pragma once
#include <iostream>

class Wire
{
public:
	Wire(bool initial_state = false);

	void Set(bool new_state) { state = new_state; }
	
	bool On() const { return state; }

	friend std::ostream& operator<<(std::ostream& os, const Wire& dt);

	static const Wire OFF;
	static const Wire ON;
	static int WireCount() { return id_counter; }

private:
	static int id_counter;
	int id;
	bool state;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);
