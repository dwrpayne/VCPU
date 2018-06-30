#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "MultiGate.h"


class TriState : public Component
{
public:
	void Connect(const Wire& in, const Wire& e);
	void Update();

	const Wire& Out() const { return out; }

private:
	const Wire* input;
	const Wire* enable;
	Wire out;
};

class TriStateEnLo : public Component
{
public:
	void Connect(const Wire& in, const Wire& e);
	void Update();

	const Wire& Out() const { return out; }

private:
	const Wire* input;
	const Wire* enable;
	Wire out;
};


template <unsigned int N>
class TriStateN : public Component
{
public:
	void Connect(const Bundle<N>& in, const Wire& e);
	void Update();

	const Bundle<N>& Out() const { return tristates.Out(); }

private:
	MultiGate<TriState, N> tristates;
};

template<unsigned int N>
inline void TriStateN<N>::Connect(const Bundle<N>& in, const Wire & e)
{
	tristates.Connect(in, Bundle<N>(e));
}

template<unsigned int N>
inline void TriStateN<N>::Update()
{
	tristates.Update();
}
