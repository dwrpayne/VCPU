#pragma once
#include <array>
#include <algorithm> 

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "Junction.h"

template <unsigned int N>
class Bus : public Component
{
public:
	Bus()
	{
		for (int i = 0; i < N; i++)
		{
			out.Connect(i, state[i]);
		}
	}
	
	template <unsigned int M, typename = typename std::enable_if<M <= N, const >::type>
	void Connect(const Bundle<M>& bundle, int start = 0)
	{
		assert(start + M <= N);
		for (int i = 0; i < M; i++)
		{
			inputWires[i + start].push_back(&bundle[i]);
		}
	}
	
	template <unsigned int M, typename = typename std::enable_if<M <= N, const >::type>
	void Remove(const Bundle<M>& bundle, int start = 0)
	{
		assert(start + M <= N);
		for (int i = 0; i < M; i++)
		{
			auto& vec = inputWires[i + start];
			vec.erase(std::remove(vec.begin(), vec.end(), &bundle[i]), vec.end());
		}
	}
	void Update()
	{
		for (int i = 0; i < N; i++)
		{
			state[i].Set(std::any_of(inputWires[i].begin(), inputWires[i].end(), [](const Wire* wire) {return wire->On(); }));
#ifdef DEBUG
			char on = 0;
			for (const auto* wire : inputWires[i])
			{
				on += wire->On() ? 1 : 0;
			}
			assert(on < 2);
#endif
		}
	}
	const Bundle<N>& Out() const { return out; }

private:
	std::array<Wire, N> state;
	std::array<std::vector<const Wire*>, N> inputWires;
	Bundle<N> out;
};


template <unsigned int N>
class StatelessBus
{
public:
	StatelessBus() {}
	StatelessBus(std::initializer_list<const Junction*> list)
	{
		Connect(list);
	}

	operator Bundle<N>()
	{
		Bundle<N> b;
		for (int i = 0; i < N; i++)
		{
			b.Connect(i, wires[i]);
		}
		return b;
	}

	template <unsigned int M>
	void Connect(const Bundle<M>& wires, int start = 0)
	{
		for (int i = 0; i < M; i++)
		{
			Connect(start + i, wires[i]);
		}
	}

	template <unsigned int M>
	void Remove(const Bundle<M>& wires, int start = 0)
	{
		for (int i = 0; i < M; i++)
		{
			Disconnect(start + i, wires[i]);
		}
	}

	void Connect(int n, const Wire& wire)
	{
		wires[n].Connect(wire);
	}

	void Disconnect(int n, const Wire& wire)
	{
		wires[n].Disconnect(wire);
	}

	const Bundle<N> Out() { return Bundle<N>(*this); }

protected:
	std::array<Junction, N> wires;
};

