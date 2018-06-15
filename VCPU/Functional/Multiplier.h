#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "FullAdder.h"
#include "NandGate.h"

// Implementation of a Baugh-Wooley signed integer multiplier: http://www.ece.uvic.ca/~fayez/courses/ceng465/lab_465/project2/multiplier.pdf

template<typename GateType>
class BaughWooleyCell : public Component
{
public:
	void Connect(const Wire& a, const Wire& b, const Wire& c, const Wire& s)
	{
		gate.Connect(a, b);
		adder.Connect(a, s, c);
	}
	void Update()
	{
		gate.Update();
		adder.Update();
	}
	const Wire& S() { return adder.S(); }
	const Wire& C() { return adder.Cout(); }
private:
	GateType gate;
	FullAdder adder;
};

typedef BaughWooleyCell<AndGate> BaughWooleyWhite;
typedef BaughWooleyCell<NandGate> BaughWooleyGrey;

template <unsigned int N>
class BaughWooleyMultiplier : public Component
{
public:
	BaughWooleyMultiplier();
	void Connect(const Bundle<N>& a, const Bundle<N>& b);
	void Update();

	const Bundle<N * 2>& Out() const { return out; }

private:
	std::array<std::array<Component*, N>, N> cells;

	FullAdderN<N> outAdder;
	Bundle<N * 2> out;
};

template<unsigned int N>
inline BaughWooleyMultiplier<N>::BaughWooleyMultiplier()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if ((i == N - 1) ^ (j == N - 1))
			{
				cells[i][j] = new BaughWooleyGrey();
			}
			else
			{
				cells[i][j] = new BaughWooleyWhite();
			}
		}
	}

	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, cells[0][i].S());
	}
	out.Connect(N, outAdder.Out());
}

template<unsigned int N>
inline void BaughWooleyMultiplier<N>::Connect(const Bundle<N>& a, const Bundle<N>& b)
{
	Bundle<N> sums, carries;
	for (int i = 0; i < N - 1; i++)
	{
		cells[i][0]->Connect(a[i], b[0], Wire::OFF, Wire::OFF);
		for (int j = 1; j < N; j++)
		{
			cells[i][j]->Connect(a[i], b[j], cells[i][j - 1]->C(), cells[i + 1][j - 1]->S());
		}
		sum.Connect(i, cells[i + 1][N - 1]->S());
		carries.Connect(i, cells[i][N - 1]->C());
	}
	for (int j = 1; j < N; j++)
	{
		cells[N - 1][j]->Connect(a[N - 1], b[j], cells[i][j - 1]->C(), Wire::OFF);
	}
	sum.Connect(N - 1, Wire::ON);
	carries.Connect(N - 1, cells[N - 1][N - 1]->C());

	outAdder.Connect(sums, carries, Wire::ON);
}

template<unsigned int N>
inline void BaughWooleyMultiplier<N>::Update()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			cells[j][i]->Update();
		}
	}
	outAdder.Update();
}

