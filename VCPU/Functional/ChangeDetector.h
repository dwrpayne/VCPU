#pragma once
#include <array>

#include "Component.h"
#include "Bundle.h"
#include "AndGate.h"
#include "OrGate.h"
#include "Register.h"
#include "Matcher.h"
#include "EdgeDetector.h"
#include "TriStateBuffer.h"

// This thing takes a Bundle and a Wire, and it outputs a wire. Output is as follows:
// input 0 -> output 0
// input 1 -> 
//		If input wire or bundle has changed from the previous update, output 1
//		Otherwise, output 0

template <unsigned int N>
class ChangeDetector : public Component
{
public:
	void Connect(const Bundle<N>& in, const Wire& on)
	{
		lastInMatcher.Connect(lastIn.Out(), in);
		lastIn.Connect(in, Wire::ON);
		onRise.Connect(on);
		eitherChange.Connect(lastInMatcher.NoMatch(), onRise.Rise());
		onAndChange.Connect(eitherChange.Out(), on);
	}
	void Update()
	{
		lastInMatcher.Update();
		lastIn.Update();
		onRise.Update();
		eitherChange.Update();
		onAndChange.Update();
	}

	const Wire& Out() { return onAndChange.Out(); }

private:
	Register<N> lastIn;
	Matcher<N> lastInMatcher;
	EdgeDetector onRise;
	OrGate eitherChange;
	AndGate onAndChange;
};

// Like a ChangeDetector, but takes two input Bundles and outputs 1 if either of them changed.
template <unsigned int N1, unsigned int N2>
class ChangeDetector2 : public Component
{
public:
	void Connect(const Bundle<N1>& in1, const Bundle<N2>& in2, const Wire& on)
	{
		lastInMatcher1.Connect(lastIn1.Out(), in1);
		lastIn1.Connect(in1, Wire::ON);
		lastInMatcher2.Connect(lastIn2.Out(), in2);
		lastIn2.Connect(in2, Wire::ON);
		onRise.Connect(on);
		eitherChange.Connect({ &lastInMatcher1.NoMatch(), &lastInMatcher2.NoMatch(), &onRise.Rise() });
		onAndChange.Connect(eitherChange.Out(), on);
	}
	void Update()
	{
		lastInMatcher1.Update();
		lastIn1.Update();
		lastInMatcher2.Update();
		lastIn2.Update();
		onRise.Update();
		eitherChange.Update();
		onAndChange.Update();
	}

	const Wire& Out() { return onAndChange.Out(); }

private:
	Register<N1> lastIn1;
	Matcher<N1> lastInMatcher1;
	Register<N2> lastIn2;
	Matcher<N2> lastInMatcher2;
	EdgeDetector onRise;
	OrGateN<3> eitherChange;
	AndGate onAndChange;
};
