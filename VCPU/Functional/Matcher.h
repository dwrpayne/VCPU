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
	const Wire& NoMatch() const { return notEqual.Out(); }
	
private:
	MultiGate<XNorGate, N> bitsxnor;
	AndGateN<N> equalAnd;
	Inverter notEqual;
};

template<unsigned int N>
inline void Matcher<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	bitsxnor.Connect(a, b);
	equalAnd.Connect(bitsxnor.Out());
	notEqual.Connect(equalAnd.Out());
}

template<unsigned int N>
inline void Matcher<N>::Update()
{
	bitsxnor.Update();
	equalAnd.Update();
	notEqual.Update();
}


template <unsigned int N, unsigned int Nreg>
class MatcherN : public Component
{
public:
	void Connect(const Bundle<N>& a, const std::array<Bundle<N>, Nreg>& candidates);
	void Update();

	const Bundle<Nreg>& Out() const { return out; }
	const Wire& AnyMatch() const { return and.Out(); }

private:
	std::array<Matcher<N>, Nreg> matchers;
	Bundle<Nreg> out;
	AndGateN<Nreg> and;
};

template<unsigned int N, unsigned int Nreg>
inline void MatcherN<N, Nreg>::Connect(const Bundle<N>& a, const std::array<Bundle<N>, Nreg>& candidates)
{
	for (int i = 0; i < Nreg; i++)
	{
		matchers[i].Connect(a, candidates[i]);
		out.Connect(i, matchers[i].Out());
	}
	and.Connect(out);
}

template<unsigned int N, unsigned int Nreg>
inline void MatcherN<N, Nreg>::Update()
{
	for (auto& m : matchers)
	{
		m.Update();
	}
	and.Update();
}



template <unsigned int N>
class NonZeroMatcher : public Component
{
public:
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<N>& match, const Wire& enable);
	void Update();
	const Wire& Match() { return matchAnd.Out(); }

private:
	Matcher<N> aMatch;
	Matcher<N> bMatch;
	OrGateN<N> nonZero;
	OrGate matchOr;
	AndGateN<3> matchAnd;
};

template<unsigned int N>
inline void NonZeroMatcher<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<N>& match, const Wire& enable)
{
	aMatch.Connect(a, match);
	bMatch.Connect(b, match);
	nonZero.Connect(match);
	matchOr.Connect(aMatch.Out(), bMatch.Out());
	matchAnd.Connect({ &matchOr.Out(), &nonZero.Out(), &enable });
}

template<unsigned int N>
inline void NonZeroMatcher<N>::Update()
{
	aMatch.Update();
	bMatch.Update();
	nonZero.Update();
	matchOr.Update();
	matchAnd.Update();
}


template <>
class Matcher<1> : public Component
{
public:
	void Connect(const Bundle<1>& a, const Bundle<1>& b);
	void Update();

	const Wire& Out() const { return match.Out(); }

private:
	XNorGate match;
};

inline void Matcher<1>::Connect(const Bundle<1>& a, const Bundle<1>& b)
{
	match.Connect(a[0], b[0]);
}

inline void Matcher<1>::Update()
{
	match.Update();
}

template <>
class Matcher<0> : public Component
{
public:
	void Connect(const Bundle<0>& a, const Bundle<0>& b);
	void Update();

	const Wire& Out() const { return Wire::ON; }
};

inline void Matcher<0>::Connect(const Bundle<0>& a, const Bundle<0>& b)
{
}

inline void Matcher<0>::Update()
{
}
