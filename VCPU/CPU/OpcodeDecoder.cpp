#include "OpcodeDecoder.h"

void OpcodeDecoder::Connect(const Bundle<6>& opcode, const Bundle<6>& func)
{
	inv.Connect(opcode);
	rFormat.Connect(inv.Out());
	loadstore.Connect(inv.Out()[4], opcode[5]);
	loadOp.Connect(inv.Out()[3], loadstore.Out());
	storeOp.Connect(opcode[3], loadstore.Out());
	branchOp.Connect({ &opcode[2], &inv.Out()[3], &inv.Out()[4], &inv.Out()[5] });
	immOp.Connect({ &opcode[3], &inv.Out()[4], &inv.Out()[5] });
	aluBImm.Connect({ &loadOp.Out(), &storeOp.Out(), &immOp.Out() });
	regWrite.Connect({ &rFormat.Out(), &loadOp.Out(), &immOp.Out()});

	zeroOpcode.Connect(opcode);
	funcOpMux.Connect({ opcode.Range<0,3>(), func.Range<0,3>() }, zeroOpcode.Out());

	branchInv.Connect(branchOp.Out());
	loadstoreInv.Connect(loadstore.Out());

	func1Inv.Connect(funcOpMux.Out()[1]);
	func2Inv.Connect(funcOpMux.Out()[2]);
	mathOp.Connect({ &func2Inv.Out(), &branchOp.Out(), &loadstore.Out() });
	addOr.Connect(loadstore.Out(), func1Inv.Out());
	subOr.Connect(funcOpMux.Out()[1], branchOp.Out());
	addOp.Connect(addOr.Out(), branchInv.Out());
	subOp.Connect(subOr.Out(), loadstoreInv.Out());

	Bundle<4> mathControl({ &subOp.Out(), &addOp.Out(), &Wire::ON, &Wire::OFF });
	Bundle<4> logicControl({ &funcOpMux.Out()[0], &funcOpMux.Out()[1], &Wire::OFF, &Wire::ON });

	control.Connect({ logicControl, mathControl }, mathOp.Out());
}

void OpcodeDecoder::Update()
{
	inv.Update();
	rFormat.Update();
	loadstore.Update();
	loadOp.Update();
	storeOp.Update();
	branchOp.Update();
	immOp.Update();
	aluBImm.Update();
	regWrite.Update();

	zeroOpcode.Update();
	funcOpMux.Update();
	branchInv.Update();
	loadstoreInv.Update();
	func1Inv.Update();
	func2Inv.Update();
	mathOp.Update();
	addOr.Update();
	subOr.Update();
	addOp.Update();
	subOp.Update();
	control.Update();
}
