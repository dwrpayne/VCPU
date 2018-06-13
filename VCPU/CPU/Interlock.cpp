#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2,
	const RegBundle& writingReg, const Wire& loadop, const Bundle<6>& opcodeIF)
{
	r1match.Connect(readR1, writingReg);
	r2match.Connect(readR2, writingReg);
	regmatch.Connect(r1match.Out(), r2match.Out());
	regNonZero.Connect(writingReg);
	branchopnor.Connect(opcodeIF.Range<3>(3));
	branchopand.Connect(opcodeIF[2], branchopnor.Out());
	loadorbranch.Connect(branchopand.Out(), loadop);
	bubble.Connect({ &regmatch.Out(), &loadorbranch.Out(), &regNonZero.Out() });
	bubbleInv.Connect(bubble.Out());
	freeze.Connect(inscachemiss, cachemiss);
	freezeInv.Connect(freeze.Out());
	freezeOrBubbleInv.Connect(freeze.Out(), bubble.Out());
}

void Interlock::Update()
{
	r1match.Update();
	r2match.Update();
	regmatch.Update();
	regNonZero.Update();
	branchopnor.Update();
	branchopand.Update();
	loadorbranch.Update();
	bubble.Update();
	bubbleInv.Update();
	freeze.Update();
	freezeInv.Update();
	freezeOrBubbleInv.Update();
}

