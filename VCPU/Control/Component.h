#pragma once

// Base class for all VCPU components

class Component
{
public:
	Component();
	~Component();

	virtual void Update() = 0;
};


template <unsigned int N>
extern constexpr int bits = 1 + bits<N / 2>;

template<>
extern constexpr int bits<2> = 1;