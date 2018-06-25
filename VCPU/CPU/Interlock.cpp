#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss,
	const RegBundle& readR1IFID, const RegBundle& readR2IFID, const RegBundle& writingRegIDEX, const Bundle<6>& opcodeIF,
	const RegBundle& readR1IDEX, const RegBundle& readR2IDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM)
{
	branchopnor.Connect(opcodeIF.Range<3>(3));
	branchopand.Connect(opcodeIF[2], branchopnor.Out());
	branchandload.Connect(branchopand.Out(), loadopEXMEM);
	idexMatcher.Connect(readR1IFID, readR2IFID, writingRegIDEX, branchopand.Out());
	exmemMatcher.Connect(readR1IDEX, readR2IDEX, writingRegEXMEM, loadopEXMEM);
	idextoexmemMatcher.Connect(readR1IFID, readR2IFID, writingRegEXMEM, branchandload.Out());
	bubble.Connect({ &idexMatcher.Match(), &exmemMatcher.Match(), &idextoexmemMatcher.Match() });
	freeze.Connect(inscachemiss, cachemiss);
	freezeInv.Connect(freeze.Out());
	bubbleID.Connect(idexMatcher.Match(), freezeInv.Out());
	exmemMatchEither.Connect(exmemMatcher.Match(), idextoexmemMatcher.Match());
	bubbleEX.Connect(exmemMatchEither.Out(), freezeInv.Out());
	proceedID.Connect(freeze.Out(), exmemMatchEither.Out());
	proceedIF.Connect(freeze.Out(), bubble.Out());
}

void Interlock::Update()
{
	branchopnor.Update();
	branchopand.Update();
	branchandload.Update();
	idexMatcher.Update();
	exmemMatcher.Update();
	idextoexmemMatcher.Update();
	bubble.Update();
	freeze.Update();
	freezeInv.Update();
	bubbleID.Update();
	exmemMatchEither.Update();
	bubbleEX.Update();
	proceedIF.Update();
	proceedID.Update();
}

