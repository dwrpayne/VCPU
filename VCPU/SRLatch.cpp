#include "SRLatch.h"

void SRLatch::Connect(const Wire& s, const Wire& r)
{
	norr.Connect(r, nors.Out());
	nors.Connect(norr.Out(), s);
}

void SRLatch::Update()
{
	norr.Update();
	nors.Update();
	norr.Update();
}
