#pragma once

#include "Component.h"
#include "ALU.h"

class CPU : public Component
{
public:
	void Connect();
	void Update();

private:
	Register<32> pc;
	RegisterFile<32, 32> registers;

};