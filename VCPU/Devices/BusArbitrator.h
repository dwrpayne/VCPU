#pragma once
#include <array>
#include "Component.h"
#include "SystemBus.h"
#include "NorGate.h"

class BusArbitrator : public Component
{
public:
	static const int N = SystemBus::NReqLines;
	~BusArbitrator();
	void Connect(SystemBus& bus);
	void Update();
	
private:
	SystemBus* pSystemBus;

	NorGate busFree;
	std::array<AndGate, N> reqs;
	std::array<AndGate, N> nextGrants;
	std::array<Inverter, N> nextGrantInvs;
	std::array<AndGate, N> outGrants;
};

