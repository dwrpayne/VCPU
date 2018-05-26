#pragma once

#include "Bundle.h"
#include "Component.h"
#include "Inverter.h"
#include "OrGate.h"
#include "Multiplexer.h"
#include "AndGate.h"

class BranchDetector : public Component
{
public:
	void Connect(const Wire& aluzero, const Wire& aluneg, const Bundle<2>& branchsel, const Wire& enable);
	void Update();
	const Wire& Out() const { return branchTakenAnd.Out(); }

private:
	Inverter aluZeroInv;
	OrGate aluNegOrZero;
	Inverter aluPos;
	Multiplexer<4> branchTakenMux;
	AndGate branchTakenAnd;
};

