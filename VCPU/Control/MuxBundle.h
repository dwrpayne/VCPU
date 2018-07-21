#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Decoder.h"
#include "Selector.h"
#include "MultiGate.h"
#include "OrGate.h"
#include "TriStateBuffer.h"
#include <type_traits>

template <unsigned int N, unsigned int Ninput>
class MuxBundle : public Component
{
public:
	typedef Bundle<N> DataBundle;
	typedef typename std::conditional<Ninput == 2, MultiGate<OrGate, N>, 
							MultiGateN<OrGateN<Ninput>, N, Ninput> >::type MultiOrGate;

	static const unsigned int BITS = bits(Ninput);
	void Connect(const std::array<DataBundle, Ninput> in, const Bundle<BITS>& sel);

	template <typename = typename std::enable_if<BITS == 1, const>::type>
	void Connect(const std::array<DataBundle, Ninput> in, const Wire& sel)
	{
		Connect(in, Bundle<BITS>(sel));
	}

	void Update();

	const DataBundle& Out() const { return select.Out(); }

private:
#if DEBUG || 1
	std::array<DataBundle, Ninput> in_bundles;
	Bundle<BITS> in_select;
#endif
	Decoder<Ninput> decoder;
	Selector<N, Ninput> select;
};

template <unsigned int N, unsigned int Ninput>
inline void MuxBundle<N, Ninput>::Connect(const std::array<DataBundle, Ninput> in, const Bundle<BITS>& sel)
{
#if DEBUG || 1
	in_bundles = in;
	in_select = sel;
#endif

	decoder.Connect(sel, Wire::ON);
	select.Connect(in, decoder.Out());
}

template <unsigned int N, unsigned int Ninput>
inline void MuxBundle<N, Ninput>::Update()
{
	decoder.Update();
	select.Update();
}


template <unsigned int N>
class MuxBundle<N, 1> : public Component
{
public:
	typedef Bundle<N> DataBundle;

	void Connect(const std::array<DataBundle, 1> in, const Bundle<0>& sel);	
	void Update();
	const DataBundle& Out() const { return buffer.Out(); }

private:
	MultiGate<TriState, N> buffer;
};

template <unsigned int N>
inline void MuxBundle<N, 1>::Connect(const std::array<DataBundle, 1> in, const Bundle<0>& sel)
{
	buffer.Connect(in[0], DataBundle::ON);
}

template <unsigned int N>
inline void MuxBundle<N, 1>::Update()
{
	buffer.Update();
}
