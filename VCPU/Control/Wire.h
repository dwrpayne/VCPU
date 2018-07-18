#pragma once
#include <iostream>

class Wire
{
public:
	Wire(bool initial_state = false);
	virtual ~Wire() {}

	inline void Set(bool new_state) { state = new_state; }
	
	virtual bool On() const { return state; }

	friend std::ostream& operator<<(std::ostream& os, const Wire& dt);

	static const Wire OFF;
	static const Wire ON;
	static int WireCount() { return id_counter; }

private:
	static unsigned int id_counter;
#ifdef DEBUG
	unsigned int id : 24;
#endif
	bool state : 8;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);
