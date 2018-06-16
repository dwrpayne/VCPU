#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2,
	const RegBundle& writingRegIDEX, const Wire& loadopIDEX, const RegBundle& writingRegEXMEM, const Wire& loadopEXMEM, 
	const Bundle<6>& opcodeIF)
{
	branchopnor.Connect(opcodeIF.Range<3>(3));
	branchopand.Connect(opcodeIF[2], branchopnor.Out());
	branchorloadexmem.Connect(branchopand.Out(), loadopEXMEM);
	idexMatcher.Connect(readR1, readR2, writingRegIDEX, loadopIDEX);
	exmemMatcher.Connect(readR1, readR2, writingRegEXMEM, branchorloadexmem.Out());
	bubble.Connect(idexMatcher.Match(), exmemMatcher.Match());
	bubbleInv.Connect(bubble.Out());
	freeze.Connect(inscachemiss, cachemiss);
	freezeInv.Connect(freeze.Out());
	freezeOrBubbleInv.Connect(freeze.Out(), bubble.Out());
}

void Interlock::Update()
{
	branchopnor.Update();
	branchopand.Update();
	branchorloadexmem.Update();
	idexMatcher.Update();
	exmemMatcher.Update();
	bubble.Update();
	bubbleInv.Update();
	freeze.Update();
	freezeInv.Update();
	freezeOrBubbleInv.Update();
}

