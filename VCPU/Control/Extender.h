#pragma once

#include "Bundle.h"
#include "Component.h"
#include "Multiplexer.h"

template <unsigned int FROM, unsigned int TO>
class Extender : public Component
{
public:
	Extender();
	void Connect(const Bundle<FROM>& from, const Wire& signext);
	void Update();

	const Bundle<TO>& Out() const { return out; }

private:
	Multiplexer<2> zeroOrSignMux; 
	Bundle<TO> out;
};


template<unsigned int FROM, unsigned int TO>
inline Extender<FROM, TO>::Extender()
	: out(zeroOrSignMux.Out())
{
}

template<unsigned int FROM, unsigned int TO>
inline void Extender<FROM, TO>::Connect(const Bundle<FROM>& from, const Wire& signext)
{
	out.Connect(0, from);
	zeroOrSignMux.Connect({ &Wire::OFF, &from[FROM - 1] }, signext);
}

template<unsigned int FROM, unsigned int TO>
inline void Extender<FROM, TO>::Update()
{
	zeroOrSignMux.Update();
}
