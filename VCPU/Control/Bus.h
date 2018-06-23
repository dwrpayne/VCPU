#pragma once
#include <array>
#include <algorithm> 

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"

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
			for (const auto& wire : inputWires[i])
			{
				on += wire.On() ? 0 : 1;
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

