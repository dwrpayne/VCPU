#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss, const Wire& haltExOp,
	const RegBundle& readR1IFID, const RegBundle& readR2IFID, const RegBundle& writingRegIDEX, const Bundle<6>& opcodeIF, const Bundle<6>& funcIF,
	const RegBundle& readR1IDEX, const RegBundle& readR2IDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM, const Wire& storeopIDEX)
{
	func3Inv.Connect(funcIF[3]);
	zeroOpcode.Connect(opcodeIF);
	funcIs8or9.Connect({ &funcIF[5], &funcIF[4], &func3Inv.Out(), &funcIF[2], &funcIF[1] });
	jumpOp.Connect(zeroOpcode.Out(), funcIs8or9.Out());
	branchopnor.Connect(opcodeIF.Range<3>(3));
	branchopand.Connect(opcodeIF[2], branchopnor.Out());
	branchOrJumpReg.Connect(branchopand.Out(), jumpOp.Out());
	branchandload.Connect(branchOrJumpReg.Out(), loadopEXMEM);
	notStoreOp.Connect(storeopIDEX);
	loadAndNotStore.Connect(loadopEXMEM, notStoreOp.Out());
	idexMatcher.Connect(readR1IFID, readR2IFID, writingRegIDEX, branchOrJumpReg.Out());
	exmemMatcher.Connect(readR1IDEX, readR2IDEX, writingRegEXMEM, loadAndNotStore.Out());
	idextoexmemMatcher.Connect(readR1IFID, readR2IFID, writingRegEXMEM, branchandload.Out());
	bubble.Connect({ &idexMatcher.Match(), &exmemMatcher.Match(), &idextoexmemMatcher.Match() });
	freeze.Connect({&inscachemiss, &cachemiss, &haltExOp});
	freezeInv.Connect(freeze.Out());
	bubbleID.Connect(idexMatcher.Match(), freezeInv.Out());
	exmemMatchEither.Connect(exmemMatcher.Match(), idextoexmemMatcher.Match());
	bubbleEX.Connect(exmemMatchEither.Out(), freezeInv.Out());
	proceedID.Connect(freeze.Out(), exmemMatchEither.Out());
	proceedIF.Connect(freeze.Out(), bubble.Out());
}

void Interlock::Update()
{
	func3Inv.Update();
	zeroOpcode.Update();
	funcIs8or9.Update();
	jumpOp.Update();
	branchopnor.Update();
	branchopand.Update();
	branchOrJumpReg.Update();
	branchandload.Update();
	idexMatcher.Update();
	notStoreOp.Update();
	loadAndNotStore.Update();
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

