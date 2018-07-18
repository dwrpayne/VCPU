#pragma once
#include <iostream>

class Wire
{
public:
	Wire(bool initial_state = false);
	virtual ~Wire() {}

	inline void Set(bool new_state) { state = new_state; }
	
	inline bool On() const 
	{
		return dynamic ? OnDynamic() : state;
	}

	virtual bool OnDynamic() const { return state; }

	friend std::ostream& operator<<(std::ostream& os, const Wire& dt);

	static const Wire OFF;
	static const Wire ON;
	static int WireCount() { return id_counter; }


protected:
	bool dynamic : 8;

private:
	static unsigned int id_counter;
#ifdef DEBUG
	unsigned int id : 24;
#endif
	bool state : 8;
};

std::ostream& operator<<(std::ostream& os, const Wire& dt);
