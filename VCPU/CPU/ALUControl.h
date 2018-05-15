#pragma once

#include "Component.h"
#include "Wire.h"
#include "Bundle.h"
#include "OrGate.h"
#include "Inverter.h"
#include "MuxBundle.h"

/*******************************
      Opcodes we care about
	op	func	ALU	
	  -----------------
	0	10000x	0110	A + B
	0	10001x	0101	A - B
	0	100100	1000	A AND B
	0	100101	1001	A OR B
	0	100110	1010	A XOR B
	0	100111	1011	A NOR B

	// Immediate instructions
	001100		1000	A AND B
	001101		1001	A OR B
	001110		1010	A XOR B
	001111		1011	A NOR B

	// Branch instructions
	000100		0101	A - B
	000101		0101	A - B
	000110		0101	A - B
	000111		0101	A - B

	// Load/store instructions
	100xxx		0110	A + B
	101xxx		0110	A + B

if F2 or immediate: logic

Logic: ALU = 1, 0, F1, F0
Math: ALU = 0, 1, (F1 | loadstore), (~F1 | branch)

*******************************/

class ALUControl
{
public:
	void Connect(const Wire& loadstore, const Wire& branch, const Wire& immediate, const Wire& rformat, const Bundle<6>& opcode, const Bundle<6>& func);
	void Update();

	const Bundle<4>& AluControl() { return out; }

private:
	OrGate logicOp;
	MuxBundle<2, 2> logicMux;
	Inverter mathOp;
	Inverter func1Inv;
	OrGate addOp;
	OrGate subOp;
	MuxBundle<2, 2> control;
	Bundle<4> out;
};


