#pragma once
#include <array>
#include "Component.h"
#include "Wire.h"
#include "Bundle.h"


class TriState : public Component
{
public:
	void Connect(const Wire& in, const Wire& e);
	void Update();

	const Wire& Out() const { return out; }

private:
	const Wire* input;
	const Wire* enable;
	Wire out;
};

class TriStateEnLo : public Component
{
public:
	void Connect(const Wire& in, const Wire& e);
	void Update();

	const Wire& Out() const { return out; }

private:
	const Wire* input;
	const Wire* enable;
	Wire out;
};
