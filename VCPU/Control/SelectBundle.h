#pragma once

#include "Component.h"
#include "MuxBundle.h"
#include "Encoder.h"

template <unsigned int N, unsigned int Ninput>
class SelectBundle : public Component
{
public:
	void Connect(const std::array<Bundle<N>, Ninput>& in, const Bundle<Ninput>& sel);
	void Update();

	const Bundle<N>& Out() const { return mux.Out(); }
	
private:
	Encoder<Ninput> select;
	MuxBundle<N, Ninput> mux;
};

template <unsigned int N, unsigned int Ninput>
inline void SelectBundle<N, Ninput>::Connect(const std::array<Bundle<N>, Ninput>& in, const Bundle<Ninput>& sel)
{
	select.Connect(sel);
	mux.Connect(in, select.Out());
}

template <unsigned int N, unsigned int Ninput>
inline void SelectBundle<N, Ninput>::Update()
{
	select.Update();
	mux.Update();
}
