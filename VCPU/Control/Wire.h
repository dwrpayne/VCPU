#pragma once
#include <iostream>

class Wire
{
public:
	Wire(bool initial_state = false);

	inline void Set(bool new_state) { state = new_state; }
	inline void SetHiZ(bool newHiZ) { hiZ = newHiZ; }
	
	virtual bool On() const { return state; }
	bool HiZ() const { return hiZ; }

	friend std::ostream& operator<<(std::ostream& os, const Wire& dt);

	static const Wire OFF;
	static const Wire ON;
	static int WireCount() { return id_counter; }

private:
	static unsigned int id_counter;
	unsigned int id : 30;
	bool state : 1;
	bool hiZ : 1;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);
