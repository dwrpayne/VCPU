#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2,
	const RegBundle& loadingReg, const Wire& loadop)
{
	r1match.Connect(readR1, loadingReg);
	r2match.Connect(readR2, loadingReg);
	regmatch.Connect(r1match.Out(), r2match.Out());
	regNonZero.Connect(loadingReg);
	bubble.Connect({ &regmatch.Out(), &loadop, &regNonZero.Out() });
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
	bubble.Update();
	bubbleInv.Update();
	freeze.Update();
	freezeInv.Update();
	freezeOrBubbleInv.Update();
}
