#pragma once
#include <vector>
#include <algorithm> 

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"

class Junction : public Wire
{
public:
	using Wire::Wire;

	void Connect(const Wire& wire)
	{
		inputs.push_back(&wire);
	}
	void Disconnect(const Wire& wire)
	{
		inputs.erase(std::remove(inputs.begin(), inputs.end(), &wire), inputs.end());
	}

	virtual bool On() const
	{
		return std::any_of(inputs.begin(), inputs.end(), [](const Wire* wire) {return wire->On(); });
	}

private:
	std::vector<const Wire*> inputs;
};

