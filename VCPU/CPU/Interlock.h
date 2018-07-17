#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Matcher.h"
#include "OrGate.h"
#include "NorGate.h"


class Interlock : public Component
{
public:
	static const int ADDR = 5;
	typedef Bundle<ADDR> RegBundle;

	void Connect(const Wire& inscachemiss, const Wire& cachemiss, const Wire& haltExOp,								// todo: replace this with a dedicated jumpreg/branch circuit in cpu.
		const RegBundle& readR1IFID, const RegBundle& readR2IFID, const RegBundle& writingRegIDEX, const Bundle<6>& opcodeIF, const Bundle<6>& funcIF,
		const RegBundle& readR1IDEX, const RegBundle& readR2IDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM, const Wire& storeopIDEX);
	void Update();

	const Wire& BubbleID() { return bubbleID.Out(); }
	const Wire& BubbleEX() { return bubbleEX.Out(); }
	const Wire& Freeze() { return freeze.Out(); }
	const Wire& ProceedEX() { return freezeInv.Out(); }
	const Wire& ProceedMEM() { return freezeInv.Out(); }
	const Wire& ProceedIF() { return proceedIF.Out(); }
	const Wire& ProceedID() { return proceedID.Out(); }

private:
	NonZeroMatcher<ADDR> idexMatcher;
	NonZeroMatcher<ADDR> exmemMatcher;
	NonZeroMatcher<ADDR> idextoexmemMatcher;
	Inverter func3Inv;
	NorGateN<6> zeroOpcode;
	NorGateN<5> funcIs8or9;
	AndGate jumpOp;	
	NorGateN<3> branchopnor;
	AndGate branchopand;
	OrGate branchOrJumpReg;
	AndGate branchandload;
	Inverter notStoreOp;
	AndGate loadAndNotStore;
	OrGateN<3> bubble;
	OrGateN<3> freeze;
	Inverter freezeInv;
	AndGate bubbleID;
	OrGate idexMatchEither;
	AndGate bubbleEX;
	NorGate proceedIF;
	NorGate proceedID;
};
