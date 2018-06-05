#pragma once

#include "Component.h"
#include "OrGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "Bundle.h"
#include "MuxBundle.h"


/*******************************
	VCPU implemented opcode table
	DEC 543210	Func	ALU		mn		Name
	--------------------------
	0	000000	100000	0110	ADD		Add
	0	000000	100010	0101	SUB		Subtract
	0	000000	100100	1000	AND		And
	0	000000	100101	1001	OR		Or
	0	000000	100110	1010	XOR		Exclusive Or
	0	000000	100111	1011	NOR		Nor

	0	000000	10101x			SLT		Set on Less Than

	2	000010					J		Jump
	3	000011					JAL		Jump and Link
	4	000100			0101	BEQ		Branch on Equal
	5	000101			0101	BNE		Branch on Not Equal
	6	000110			0101	BLEZ	Branch on Less Than or Equal to Zero
	7	000111			0101	BGTZ	Branch on Greater Than Zero

	8	001000			0110	ADDI	Add Immediate
	9	001001			0110	ADDUI	Add Immediate
	10	001010			0101	SLTI	Set on Less Than Immediate
	11	001011			0101	SLTUI	Set on Less ThanImmediate
	12	001100			1000	ANDI	And Immediate
	13	001101			1001	ORI		Or Immediate
	14	001110			1010	XORI	Exclusive Or Immediate
	15	001111			????	LUI		Load Upper Immediate

	32-39  100xxx		0110	LW		Load Word 
	40-47	101xxx		0110	SW		Store Word
	
*******************************/

class OpcodeDecoder : public Component
{
public:
	OpcodeDecoder();
	void Connect(const Bundle<6>& opcode, const Bundle<6>& func);
	void Update();

	static const int OUT_WIDTH = 14;
	
	class OpcodeDecoderBundle : public Bundle<OUT_WIDTH>
	{
	public:
		OpcodeDecoderBundle() 
			: Bundle<OUT_WIDTH>()
		{}
		OpcodeDecoderBundle(std::initializer_list<const Wire*> list)
			: Bundle<OUT_WIDTH>(list)
		{}
		OpcodeDecoderBundle(const Bundle<OUT_WIDTH>& other)
			: Bundle<OUT_WIDTH>(other)
		{}

		const Wire& Branch() const { return Get(0); }
		const Wire& LoadOp() const { return Get(1); }
		const Wire& StoreOp() const { return Get(2); }
		const Wire& RFormat() const { return Get(3); }
		const Wire& AluBFromImm() const { return Get(4); }
		const Wire& RegWrite() const { return Get(5); }
		const Wire& SltOp() const { return Get(6); }
		const Wire& ShiftOp() const { return Get(7); }
		const Wire& ShiftAmtOp() const { return Get(8); }
		const Wire& Halt() const { return Get(9); }
		const Wire& JumpOp() const { return Get(10); }
		const Wire& JumpLink() const { return Get(11); }
		const Bundle<2> BranchSel() const { return Range<2>(12); }
	};

	const Wire& Branch() const { return out.Branch(); }
	const Wire& LoadOp() const { return out.LoadOp(); }
	const Wire& StoreOp() const { return out.StoreOp(); }
	const Wire& RFormat() const { return out.RFormat(); }
	const Wire& AluBFromImm() const { return out.AluBFromImm(); }
	const Wire& RegWrite() const { return out.RegWrite(); }
	const Wire& SltOp() const { return out.SltOp(); }
	const Wire& ShiftOp() const { return out.ShiftOp(); }
	const Wire& ShiftAmtOp() const { return out.ShiftAmtOp(); }
	const Bundle<2> BranchSel() const { return out.BranchSel(); }
	const OpcodeDecoderBundle& OutBundle() { return out; }

	const Bundle<4>& AluControl() { return controlAll.Out(); }

private:
	InverterN<6> inv;
	AndGateN<6> rFormat;
	AndGate loadstore;
	AndGate loadOp;
	AndGate storeOp;
	AndGateN<4> branchOp;
	AndGateN<3> immOp;
	NorGateN<4> shiftOp;
	NorGateN<5> shiftAmtOp;
	AndGateN<5> jumpOp;
	AndGateN<5> jumpReg;
	AndGate jumpLink;
	OrGateN<3> aluBImm;
	OrGateN<3> regWrite;

	OrGateN<6> nonzeroOpcode;
	Inverter zeroOpcode;
	MuxBundle<6, 2> funcOpMux;
	Inverter func1Inv;
	Inverter func2Inv;
	Inverter func4Inv;
	Inverter branchInv;
	Inverter loadstoreInv;
	AndGateN<5> sltop;
	OrGateN<3> mathOp;
	OrGate addOr;
	OrGate subOr;
	AndGate addOp;
	AndGate subOp;
	MuxBundle<4, 2> control;
	MuxBundle<4, 2> controlAll;
	AndGateN<6> halt;

	OpcodeDecoderBundle out;
};



/*******************************
		VCPU opcode table
	DEC 543210	Func	mn		Name
	-----------------
	0	000000	000000	SLL		Shift Left Logical
	0	000000	000010	SRL		Shift Right Logical
	0	000000	000011	SRA		Shift Right Arithmetic
	0	000000	000100	SLLV	Shift Left Logical Var
	0	000000	000110	SRLV	Shift Right Logical Var
	0	000000	000111	SRAV	Shift Right Arithmetic Var

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