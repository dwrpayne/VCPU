#include "SRLatch.h"


SRLatch::SRLatch(const Wire& s, const Wire& r)
	: norr(r, WIRE_DISCONNECTED)
	, nors(WIRE_DISCONNECTED, s)
{
	Connect(s,r);
}

void SRLatch::Connect(const Wire& s, const Wire& r)
{
	norr.Connect(r, nors.Out());
	nors.Connect(norr.Out(), s);
	Update();
}

void SRLatch::Update()
{
	norr.Update();
	nors.Update();
	norr.Update();
}
