#pragma once

#include "Component.h"
#include "OrGate.h"
#include "NandGate.h"
#include "Bundle.h"

class OpcodeDecoder : public Component
{
public:
	void Connect(const Bundle<6>& opcode);
	void Update();

	const Wire& Branch() { return beq.Out(); }
	const Wire& MemToReg() { return lw.Out(); }
	const Wire& RegTorD() { return rFormat.Out(); }
	const Wire& MemWrite() { return sw.Out(); }
	const Wire& AluBSrc() { return aluSrc.Out(); }
	const Wire& RegWrite() { return regWrite.Out(); }

private:
	InverterN<6> inv;
	AndGateN<6> rFormat;
	AndGateN<6> lw;
	AndGateN<6> sw;
	AndGateN<6> beq;
	OrGate aluSrc;
	OrGate regWrite;
};