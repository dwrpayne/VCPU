#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss,
	const RegBundle& readR1IFID, const RegBundle& readR2IFID, const RegBundle& writingRegIDEX, const Bundle<6>& opcodeIF,
	const RegBundle& readR1IDEX, const RegBundle& readR2IDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM)
{
	branchopnor.Connect(opcodeIF.Range<3>(3));
	branchopand.Connect(opcodeIF[2], branchopnor.Out());
	idexMatcher.Connect(readR1IFID, readR2IFID, writingRegIDEX, branchopand.Out());
	exmemMatcher.Connect(readR1IDEX, readR2IDEX, writingRegEXMEM, loadopEXMEM);
	bubble.Connect(idexMatcher.Match(), exmemMatcher.Match());
	freeze.Connect(inscachemiss, cachemiss);
	freezeInv.Connect(freeze.Out());
	bubbleID.Connect(idexMatcher.Match(), freezeInv.Out());
	bubbleEX.Connect(exmemMatcher.Match(), freezeInv.Out());
	proceedID.Connect(freeze.Out(), exmemMatcher.Match());
	proceedIF.Connect(freeze.Out(), bubble.Out());
}

void Interlock::Update()
{
	branchopnor.Update();
	branchopand.Update();
	idexMatcher.Update();
	exmemMatcher.Update();
	bubble.Update();
	freeze.Update();
	freezeInv.Update();
	bubbleID.Update();
	bubbleEX.Update();
	proceedIF.Update();
	proceedID.Update();
}

