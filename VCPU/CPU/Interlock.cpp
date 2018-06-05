#include "Interlock.h"


void Interlock::Connect(const Wire& inscachemiss, const Wire& cachemiss, const RegBundle& readR1, const RegBundle& readR2,
	const RegBundle& loadingReg, const Wire& loadop)
{
	r1match.Connect(readR1, loadingReg);
	r2match.Connect(readR2, loadingReg);
	regmatch.Connect(r1match.Out(), r2match.Out());
	stallRAW.Connect(regmatch.Out(), loadop);
	bubble.Connect({ &inscachemiss, &cachemiss, &stallRAW.Out() });
	bubbleInv.Connect(bubble.Out());
}

void Interlock::Update()
{
	r1match.Update();
	r2match.Update();
	regmatch.Update();
	stallRAW.Update();
	bubble.Update();
	bubbleInv.Update();
}
