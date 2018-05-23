#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Inverter.h"
#include "AndGate.h"
#include "OrGate.h"


template <unsigned int N>
class Multiplexer : public Component
{
public:
	static const unsigned int BITS = bits(N);
	void Connect(const Bundle<N>& in, const Bundle<BITS>& sel);
	void Update();

	const Wire& Out() const { return muxOut.Out(); }

private:
	Multiplexer<N/2> mux0;
	Multiplexer<N/2> mux1;
	Multiplexer<2> muxOut;
};

template <unsigned int N>
inline void Multiplexer<N>::Connect(const Bundle<N>& in, const Bundle<BITS>& sel)
{
	const Bundle<BITS-1> local_sel = sel.Range<BITS - 1>(0);
	mux0.Connect(in.Range<N/2>(0), local_sel);
	mux1.Connect(in.Range<N/2>(N/2), local_sel);
	muxOut.Connect({ &mux0.Out(), &mux1.Out() }, sel.Get(BITS - 1));
}

template <unsigned int N>
inline void Multiplexer<N>::Update()
{
	mux0.Update();
	mux1.Update();
	muxOut.Update();
}


template <>
class Multiplexer<2> : public Component
{
public:
	void Connect(const Bundle<2>& in, const Wire& sel);
	void Update();
	
	const Wire& Out() const { return orOut.Out(); }

private:
	Inverter inv;
	AndGate and0;
	AndGate and1;
	OrGate orOut;
};

inline void Multiplexer<2>::Connect(const Bundle<2>& in, const Wire& sel)
{
	inv.Connect(sel);
	and0.Connect(in[0], inv.Out());
	and1.Connect(in[1], sel);
	orOut.Connect(and0.Out(), and1.Out());
}

inline void Multiplexer<2>::Update()
{
	inv.Update();
	and0.Update();
	and1.Update();
	orOut.Update();
}
