#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "FullAdder.h"
#include "NandGate.h"
#include "Register.h"

// Implementation of a Baugh-Wooley signed integer multiplier: http://www.ece.uvic.ca/~fayez/courses/ceng465/lab_465/project2/multiplier.pdf

class BaughWooleyCell : public Component
{
public:
	virtual void Connect(const Wire& a, const Wire& b, const Wire& c, const Wire& s) = 0;
	virtual void Update() = 0;
	const Wire& S() { return adder.S(); }
	const Wire& C() { return adder.Cout(); }
protected:
	FullAdder adder;
};

template<typename GateType>
class BaughWooleyCellT : public BaughWooleyCell
{
public:
	virtual void Connect(const Wire& a, const Wire& b, const Wire& c, const Wire& s)
	{
		gate.Connect(a, b);
		adder.Connect(gate.Out(), s, c);
	}
	virtual void Update()
	{
		gate.Update();
		adder.Update();
	}
private:
	GateType gate;
};

typedef BaughWooleyCellT<AndGate> BaughWooleyWhite;
typedef BaughWooleyCellT<NandGate> BaughWooleyGrey;

template <unsigned int N>
class Multiplier : public Component
{
public:
	Multiplier();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& enable);
	void Update();

	const Bundle<N>& OutLo() const { return outLo.Out(); }
	const Bundle<N>& OutHi() const { return outHi.Out(); }

private:
#if DEBUG
	Bundle<N> sums, carries;
#endif
	std::array<std::array<BaughWooleyCell*, N>, N> cells;

	FullAdderN<N> outAdder;
	Register<N> outLo;
	Register<N> outHi;
};

template<unsigned int N>
inline Multiplier<N>::Multiplier()
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
}

template<unsigned int N>
inline void Multiplier<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Wire& enable)
{
#ifndef DEBUG
	Bundle<N> sums, carries;
#endif
	for (int i = 0; i < N; i++)
	{
		cells[i][0]->Connect(a[i], b[0], Wire::OFF, Wire::OFF);
	}
	for (int i = 0; i < N - 1; i++)
	{
		for (int j = 1; j < N; j++)
		{
			cells[i][j]->Connect(a[i], b[j], cells[i][j - 1]->C(), cells[i + 1][j - 1]->S());
		}
		sums.Connect(i, cells[i + 1][N - 1]->S());
		carries.Connect(i, cells[i][N - 1]->C());
	}
	for (int j = 1; j < N; j++)
	{
		cells[N - 1][j]->Connect(a[N - 1], b[j], cells[N-1][j - 1]->C(), Wire::OFF);
	}
	sums.Connect(N - 1, Wire::ON);
	carries.Connect(N - 1, cells[N - 1][N - 1]->C());

	outAdder.Connect(sums, carries, Wire::ON);

	Bundle<N> outLoBundle;
	for (int i = 0; i < N; ++i)
	{
		outLoBundle.Connect(i, cells[0][i]->S());
	}
	outLo.Connect(outLoBundle, enable);
	outHi.Connect(outAdder.Out(), enable);
}

template<unsigned int N>
inline void Multiplier<N>::Update()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			cells[j][i]->Update();
		}
	}
	outAdder.Update();
	outLo.Update();
	outHi.Update();
}

