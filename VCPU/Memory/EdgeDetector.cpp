#include "EdgeDetector.h"

void EdgeDetector::Connect(const Wire & in)
{
	now.Connect(in, Wire::ON);
	last.Connect(now.Q(), Wire::ON);
	rise.Connect(now.Q(), last.NotQ());
	fall.Connect(now.NotQ(), last.Q());
}

void EdgeDetector::Update()
{
	last.Update();
	now.Update();
	rise.Update();
	fall.Update();
}
