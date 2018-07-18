#pragma once
#include <vector>
#include <algorithm> 

#include "Wire.h"

class Junction : public Wire
{
public:
	using Wire::Wire;

	void Connect(const Wire& wire)
	{
		if (&wire != &Wire::OFF)
		{
			inputs.push_back(&wire);
		}
	}
	void Disconnect(const Wire& wire)
	{
		if (&wire != &Wire::OFF)
		{
			inputs.erase(std::remove(inputs.begin(), inputs.end(), &wire), inputs.end());
		}
	}

	virtual bool On() const
	{
		return std::any_of(inputs.begin(), inputs.end(), [](const Wire* wire) {return wire->On(); });
	}

private:
	std::vector<const Wire*> inputs;
};

