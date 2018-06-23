#pragma once
#include <array>
#include <assert.h>
#include "Wire.h"

/***************
A Bundle is a way to collect some wires and hand them to another component.
It doesn't really exist and can't be modified. 
If you reconnect its wires, you have to reconnect the components that are using it.
****************/

template <unsigned int N>
class Bundle
{
public:
	static const unsigned int N = N;
	static const Bundle<N> OFF;
	static const Bundle<N> ON;
	static const Bundle<N> ERR;

	Bundle() {}
	Bundle(std::initializer_list<const Wire*> list)
	{
		Connect(list);
	}

	Bundle(const Bundle<N>& other)
	{
		Connect(0, other);
	}

	// Splits a single wire into an n-way Bundle.
	explicit Bundle(const Wire& wire)
	{
		for (int i = 0; i < N; ++i)
		{
			Connect(i, wire);
		}
	}

	// Creates a const Bundle (of Wire::OFF and Wire::ON) representing a number
	explicit Bundle(long long n)
	{
		InitNumber(n);
	}

	explicit Bundle(int n)
	{
		InitNumber(n);
	}

	explicit Bundle(unsigned int n)
	{
		int val = n < pow2(N) / 2 ? n : n - pow2(N);
		InitNumber(val);
	}
	
	void InitNumber(long long n)
	{
		bool negative = n < 0;
		n = abs(n) - (int)negative;

		assert(n <= pow(2, N));
		for (int i = 0; i < N - 1; i++)
		{
			bool on = negative ^ (bool)(n % 2);
			Connect(i, on ? Wire::ON : Wire::OFF);
			n /= 2;
		}
		Connect(N - 1, negative ? Wire::ON : Wire::OFF);
	}

	void Connect(std::initializer_list<const Wire*> list)
	{
		int i = 0;
		for (const auto& wire : list)
		{
			Connect(i++, *wire);
		}
	}
	
	template <unsigned int M>
	void Connect(int start, const Bundle<M>& wires)
	{
		for (int i = 0; i < M; i++)
		{
			Connect(start + i, wires[i]);
		}
	}

	void Connect(int n, const Wire& wire)
	{
		wires[n] = &wire;
	}

	template <unsigned int TO>
	const Bundle<TO> Range(unsigned int start=0) const
	{
		int shiftby = -(int)start;
		return Bundle<TO>(*this, shiftby);
	}

	// Sign-extends a Bundle to a wider one, current wires in the low bits.
	template <unsigned int TO>
	const Bundle<TO> ZeroExtend() const
	{
		return Bundle<TO>(*this);
	}

	// Sign-extends a Bundle to a wider one, current wires in the low bits.
	template <unsigned int TO>
	const Bundle<TO> SignExtend() const
	{
		return ShiftRightSignExtend<TO>(0);
	}

	// Zero-extends a bundle, shifting the current wires over by `shiftby`.
	// Asserts if there isn't enough room in the extended bundle size.
	// shiftby > 0 is a left shift (larger number)
	template <unsigned int TO>
	const Bundle<TO> ShiftZeroExtend(int shiftby) const
	{
		assert(N + shiftby <= TO);
		return ShiftZeroExtendCanLose<TO>(shiftby);
	}

	// Zero-extends a bundle, shifting the current wires over by `shiftby`.
	// May lose wires if you shift by more than you have output space for.
	// shiftby > 0 is a left shift (larger number)
	template <unsigned int TO>
	const Bundle<TO> ShiftZeroExtendCanLose(int shiftby) const
	{	
		return Bundle<TO>(*this, shiftby);
	}

	// Right-shifts, sign-extending a bundle, shifting the current wires over by `shiftby`.
	// Loses the low bits
	template <unsigned int TO>
	const Bundle<TO> ShiftRightSignExtend(int shiftby) const
	{
		return ShiftRightWireExtend<TO>(shiftby, Get(N-1));
	}

	// Right-shifts, extending a bundle, shifting the current wires over by `shiftby`.
	// Provide a fill wire.
	// Loses the low bits
	template <unsigned int TO>
	const Bundle<TO> ShiftRightWireExtend(int shiftby, const Wire& fill) const
	{
		return Bundle<TO>(*this, -shiftby, fill);
	}

	const Wire& Get(unsigned int n) const 
	{
		return *wires[n];
	}

	const Wire& operator[](unsigned int n) const 
	{
		return Get(n);
	}

	template <typename = typename std::enable_if<N==1, const >::type>
	operator const Wire&() const
	{
		return Get(0);
	}
	
	int Read() const
	{
		int n = 0;
		bool negative = Get(N-1).On();
		for (int i = N-2; i >= 0; --i)
		{
			n *= 2;
			n += (negative ^ Get(i).On()) ? 1 : 0;
		}
		return (negative ? -1 : 1) * n - (int)negative;
	}

	unsigned int UnsignedRead() const
	{
		int val = Read();
		return val >= 0 ? val : pow2(N) + val;
	}

	long long ReadLong() const
	{
		long long n = 0;
		bool negative = Get(N - 1).On();
		for (int i = N - 2; i >= 0; --i)
		{
			n *= 2;
			n += (negative ^ Get(i).On()) ? 1 : 0;
		}
		return (negative ? -1 : 1) * n - (int)negative;
	}

	template <unsigned int N>
	friend std::ostream& operator<<(std::ostream& os, const Bundle<N>& b);

protected:
	std::array<const Wire*, N> wires;

	// Creates a Bundle from another one, optionally shifting the original. 
	// Optional fill wire for wires not contained in the shifted original.
	// Private because it's not very readable. Clients should use the helper functions.	
	template <unsigned int M>
	explicit Bundle(const Bundle<M>& other, int shiftby = 0, const Wire& fill = Wire::OFF)
	{
		for (int i = 0; i < N; ++i)
		{
			int otherbit = i - shiftby;
			if (0 <= otherbit && otherbit < M)
			{
				Connect(i, other[otherbit]);
			}
			else
			{
				Connect(i, fill);
			}
		}
	}

	template <unsigned int M> friend class Bundle;
};

template<unsigned int N>
std::ostream& operator<<(std::ostream& os, const Bundle<N>& b)
{
	for (auto w : b.wires)
	{
		os << w;
	}
	return os;
}

template<unsigned int N> const Bundle<N> Bundle<N>::OFF(Wire::OFF);
template<unsigned int N> const Bundle<N> Bundle<N>::ON(Wire::ON);

template<> const Bundle<8> Bundle<8>::ERR(0xaaU);
template<> const Bundle<16> Bundle<16>::ERR(0xdeadU);
template<> const Bundle<32> Bundle<32>::ERR(0xdeadbeefU);

