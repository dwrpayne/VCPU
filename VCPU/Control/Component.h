#pragma once
#include <array>
#include <iostream>
#include "Wire.h"
// Base class for all VCPU components

class Component
{
public:
	Component(){}
	~Component() {}

	virtual void Update() = 0;
	//int prewirecount;
};

constexpr unsigned int bits(unsigned int N)
{
	if (N == 1) return 0;
	return N <= 2 ? 1 : 1 + bits(N / 2);
}

constexpr unsigned int pow2(unsigned int exp)
{
	return exp == 0 ? 1 : 2 * pow2(exp - 1);
}
