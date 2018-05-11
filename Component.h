#pragma once

// Base class for all VCPU components

class Component
{
public:
	Component();
	~Component();

	virtual void Update() = 0;
};

