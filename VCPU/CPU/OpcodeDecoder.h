#pragma once

#include "Component.h"
#include "OrGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "Bundle.h"
#include "MuxBundle.h"


/*******************************
	VCPU implemented opcode table
	DEC 543210	Func	mn		Name
	-----------------
	0	000000	100000	ADD		Add
	0	000000	100010	SUB		Subtract
	0	000000	100100	AND		And
	0	000000	100101	OR		Or
	0	000000	100110	XOR		Exclusive Or
	0	000000	100111	NOR		Nor

	0	000000	10101x	SLT		Set on Less Than

	2	000010			J		Jump
	3	000011			JAL		Jump and Link
	4	000100			BEQ		Branch on Equal
	5	000101			BNE		Branch on Not Equal
	6	000110			BLEZ	Branch on Less Than or Equal to Zero
	7	000111			BGTZ	Branch on Greater Than Zero

	8	001000			ADDI	Add Immediate
	9	001001			ADDUI	Add Immediate
	10	001010			SLTI	Set on Less Than Immediate
	11	001011			SLTUI	Set on Less ThanImmediate
	12	001100			ANDI	And Immediate
	13	001101			ORI		Or Immediate
	14	001110			XORI	Exclusive Or Immediate
	15	001111			LUI		Load Upper Immediate

	32-39  100xxx			LW		Load Word 
	40-47	101xxx			SW		Store Word
	
*******************************/

class OpcodeDecoder : public Component
{
public:
	void Connect(const Bundle<6>& opcode, const Bundle<6>& func);
	void Update();

	class OpcodeDecoderBundle : public Bundle<8>
	{
	public:
		OpcodeDecoderBundle(std::initializer_list<const Wire*> list)
			: Bundle<8>(list)
		{}
		OpcodeDecoderBundle(const Bundle<8>& other)
			: Bundle<8>(other)
		{}
		const Wire& Branch() { return Get(0); }
		const Wire& LoadStore() { return Get(1); }
		const Wire& LoadOp() { return Get(2); }
		const Wire& StoreOp() { return Get(3); }
		const Wire& RFormat() { return Get(4); }
		const Wire& IFormat() { return Get(5); }
		const Wire& AluBFromImm() { return Get(6); }
		const Wire& RegWrite() { return Get(7); }
	};

	const Wire& Branch() { return branchOp.Out(); }
	const Wire& LoadStore() { return loadstore.Out(); }
	const Wire& LoadOp() { return loadOp.Out(); }
	const Wire& StoreOp() { return storeOp.Out(); }
	const Wire& RFormat() { return rFormat.Out(); }
	const Wire& IFormat() { return immOp.Out(); }
	const Wire& AluBFromImm() { return aluBImm.Out(); }
	const Wire& RegWrite() { return regWrite.Out(); }
	const OpcodeDecoderBundle AsBundle() { return { &Branch(), &LoadStore(), &LoadOp(), &StoreOp(), &RFormat(), &IFormat(), &AluBFromImm(), &RegWrite() }; }

	const Bundle<4>& AluControl() { return control.Out(); }


private:
	InverterN<6> inv;
	AndGateN<6> rFormat;
	AndGate loadstore;
	AndGate loadOp;
	AndGate storeOp;
	AndGateN<4> branchOp;
	AndGateN<3> immOp;
	OrGateN<3> aluBImm;
	OrGateN<3> regWrite;

	NorGateN<6> zeroOpcode;
	MuxBundle<3, 2> funcOpMux;
	Inverter func1Inv;
	Inverter func2Inv;
	Inverter branchInv;
	Inverter loadstoreInv;
	OrGateN<3> mathOp;
	OrGate addOr;
	OrGate subOr;
	AndGate addOp;
	AndGate subOp;
	MuxBundle<4, 2> control;
};



/*******************************
		VCPU opcode table
	DEC 543210	Func	mn		Name
	-----------------
	0	000000	001000	JR		Jump Register
	0	000000	001001	JALR	Jump and Link Register

	0	000000	100000	ADD		Add
	0	000000	100001	ADDU	Add Unsigned
	0	000000	100010	SUB		Subtract
	0	000000	100011	SUBU	Subtract Unsigned
	0	000000	100100	AND		And
	0	000000	100101	OR		Or
	0	000000	100110	XOR		Exclusive Or
	0	000000	100111	NOR		Nor
	0	000000	101010	SLT		Set on Less Than
	0	000000	101011	SLTU	Set on Less Than Unsigned

	2	000010			J		Jump
	3	000011			JAL		Jump and Link
	4	000100			BEQ		Branch on Equal
	5	000101			BNE		Branch on Not Equal
	6	000110			BLEZ	Branch on Less Than or Equal to Zero
	7	000111			BGTZ	Branch on Greater Than Zero

	8	001000			ADDI	Add Immediate
	9	001001			ADDIU	Add Immediate Unsigned
	10	001010			SLTI	Set on Less Than Immediate
	11	001011			SLTIU	Set on Less Than Immediate Unsigned
	12	001100			ANDI	And Immediate
	13	001101			ORI		Or Immediate
	14	001110			XORI	Exclusive Or Immediate
	15	001111			LUI		Load Upper Immediate
	32	100000			LB		Load Byte
	33	100001			LH		Load Halfword
	34  100010			LWL		Load Word Left
	35  100011			LW		Load Word 
	36  100100			LBU		Load Byte Unsigned
	37  100101			LHU		Load Halfword Unsigned
	38  100110			LWR		Load Word Right
	40	101000			SB		Store Byte
	41	101001			SH		Store Halfword
	42	101010			SWL		Store Word Left
	43	101011			SW		Store Word
	46	101110			SWR		Store Word Right

C[2:0] are passed through to the Adder as those are its opcodes

*******************************/