#pragma once

#include "Component.h"
#include "Bundle.h"
#include "XNorGate.h"
#include "AndGate.h"
#include "MultiGate.h"

template <unsigned int N>
class Matcher : public Component
{
public:
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update();

	const Wire& Out() const { return equalAnd.Out(); }
	
private:
	MultiGate<XNorGate, N> bitsxnor;
	AndGateN<N> equalAnd;
};

template<unsigned int N>
inline void Matcher<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	bitsxnor.Connect(a, b);
	equalAnd.Connect(bitsxnor.Out());
}

template<unsigned int N>
inline void Matcher<N>::Update()
{
	bitsxnor.Update();
	equalAnd.Update();
}
