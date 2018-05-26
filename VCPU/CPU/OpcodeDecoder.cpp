#include "OpcodeDecoder.h"

OpcodeDecoder::OpcodeDecoder()
{
	out.Connect({ &branchOp.Out(), &loadOp.Out(), &storeOp.Out(), &rFormat.Out(),
		&aluBImm.Out(), &regWrite.Out(), &sltop.Out(), &shiftOp.Out(), 
		&funcOpMux.Out()[0], &funcOpMux.Out()[1] });
}

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
	regWrite.Connect({ &rFormat.Out(), &loadOp.Out(), &immOp.Out() });

	nonzeroOpcode.Connect(opcode);
	zeroOpcode.Connect(nonzeroOpcode.Out());
	funcOpMux.Connect({ opcode, func }, zeroOpcode.Out());
	shiftOp.Connect({ &nonzeroOpcode.Out(), &func[3], &func[4], &func[5] });

	branchInv.Connect(branchOp.Out());
	loadstoreInv.Connect(loadstore.Out());

	func1Inv.Connect(funcOpMux.Out()[1]);
	func2Inv.Connect(funcOpMux.Out()[2]);
	func4Inv.Connect(funcOpMux.Out()[4]);

	sltop.Connect({ &funcOpMux.Out()[1], &func2Inv.Out(), &funcOpMux.Out()[3], &func4Inv.Out(), &loadstoreInv.Out() });
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
	nonzeroOpcode.Update();
	zeroOpcode.Update();
	funcOpMux.Update();
	shiftOp.Update();
	branchInv.Update();
	loadstoreInv.Update();
	func1Inv.Update();
	func2Inv.Update();
	func4Inv.Update();
	sltop.Update();
	mathOp.Update();
	addOr.Update();
	subOr.Update();
	addOp.Update();
	subOp.Update();
	control.Update();
}
