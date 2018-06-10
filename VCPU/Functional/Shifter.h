#pragma once
#include <array>
#include "Component.h"
#include "MuxBundle.h"
#include "Bundle.h"


template <unsigned int N>
class RightShifter : public Component
{
public:
	static const int BITS = bits(N);
	void Connect(const Bundle<N>& in, const Bundle<BITS>& shift, const Wire& signextend);
	void Update();

	const Bundle<N>& Out() const { return muxes[BITS - 1].Out(); }

private:
	AndGate shiftinhi;
	std::array<MuxBundle<N, 2>, BITS> muxes;
};


template <unsigned int N>
class LeftShifter : public Component
{
public:
	static const int BITS = bits(N);
	void Connect(const Bundle<N>& in, const Bundle<BITS>& shift);
	void Update();

	const Bundle<N>& Out() const { return muxes[BITS - 1].Out(); }

private:
	std::array<MuxBundle<N, 2>, BITS> muxes;
};

template <unsigned int N>
class Shifter : public Component
{
public:
	static const int BITS = bits(N);
	void Connect(const Bundle<N>& in, const Bundle<BITS>& shift, const Wire& do_right, const Wire& signextend);
	void Update();

	const Bundle<N>& Out() const { return outMux.Out(); }

private:
	LeftShifter<N> left;
	RightShifter<N> right;
	MuxBundle<N, 2> outMux;
};

template<unsigned int N>
inline void Shifter<N>::Connect(const Bundle<N>& in, const Bundle<BITS>& shift, const Wire& do_right, const Wire& signextend)
{
	left.Connect(in, shift);
	right.Connect(in, shift, signextend);
	outMux.Connect({ left.Out(), right.Out() }, do_right);
}

template<unsigned int N>
inline void Shifter<N>::Update()
{
	left.Update();
	right.Update();
	outMux.Update();
}


template<unsigned int N>
inline void RightShifter<N>::Connect(const Bundle<N>& in, const Bundle<BITS>& shift, const Wire& signextend)
{
	shiftinhi.Connect(signextend, in[N - 1]);
	
	// This just simplifies the wiring math so I don't have to worry about indexing off the end of the input.
	Bundle<N> in_shift(shiftinhi.Out());
	in_shift.Connect(0, in.Range<N - 1>(1));
	muxes[0].Connect({ in, in_shift }, shift[0]);
	for (int bit = 1; bit < BITS; bit++)
	{
		unsigned int shift_by = 1 << bit;
		in_shift = Bundle<N>(shiftinhi.Out());
		for (unsigned int i = 0; i < N - shift_by; i++)
		{
			in_shift.Connect(i, muxes[bit - 1].Out()[shift_by + i]);
		}
		muxes[bit].Connect({ muxes[bit - 1].Out(), in_shift }, shift[bit]);
	}
}

template<unsigned int N>
inline void RightShifter<N>::Update()
{
	shiftinhi.Update();
	for (auto& mux : muxes)
	{
		mux.Update();
	}
}


template<unsigned int N>
inline void LeftShifter<N>::Connect(const Bundle<N>& in, const Bundle<BITS>& shift)
{
	for (int bit = 0; bit < BITS; bit++)
	{
		unsigned int shift_by = 1 << bit;
		const Bundle<N>& bundle = bit == 0 ? in : muxes[bit - 1].Out();
		Bundle<N> shifted = bundle.ShiftZeroExtendCanLose<N>(shift_by);
		muxes[bit].Connect({ bundle, shifted }, shift[bit]);
	}
}

template<unsigned int N>
inline void LeftShifter<N>::Update()
{
	for (auto& mux : muxes)
	{
		mux.Update();
	}
}
