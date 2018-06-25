#include "OpcodeDecoder.h"

OpcodeDecoder::OpcodeDecoder()
{
	out.Connect({ &branchOp.Out(), &loadOp.Out(), &storeOp.Out(), &zeroOpcode.Out(),		// 4
		&aluBImm.Out(), &regWrite.Out(), &sltop.Out(), &shiftOp.Out(),&shiftAmtOp.Out(),	// 9
		&halt.Out(), &jumpOp.Out(), &jumpLink.Out(), &jumpReg.Out(),						// 13
		&luiOp.Out(), &mathOp.Out(), &funcOpMux.Out()[0], &funcOpMux.Out()[1],				// 17
		&memOpByte.Out(), &memOpHalf.Out(), &inv.Out()[2], &jumpOrBranch.Out(),				// 21
		&multOp.Out(), &funcOpMux.Out()[1], &multMoveOp.Out(), &loadOpInv.Out()				// 24
		});
}

void OpcodeDecoder::Connect(const Bundle<6>& opcode, const Bundle<6>& func)
{
	inv.Connect(opcode);
	halt.Connect(opcode);
	zeroOpcode.Connect(inv.Out());
	loadstore.Connect(inv.Out()[4], opcode[5]);
	loadstoreInv.Connect(loadstore.Out());
	loadOp.Connect(inv.Out()[3], loadstore.Out());
	loadOpInv.Connect(loadOp.Out());
	storeOp.Connect(opcode[3], loadstore.Out());
	branchOp.Connect({ &opcode[2], &inv.Out()[3], &inv.Out()[4], &inv.Out()[5] });
	branchInv.Connect(branchOp.Out());
	immOp.Connect({ &opcode[3], &inv.Out()[4], &inv.Out()[5] });
	jumpImm.Connect({ &opcode[1], &inv.Out()[2], &inv.Out()[3], &inv.Out()[4], &inv.Out()[5] });
	luiOp.Connect({ &opcode[0], &opcode[1], &opcode[2], &opcode[3], &inv.Out()[4], &inv.Out()[5] });
	luiOpInv.Connect(luiOp.Out());

	nonzeroOpcode.Connect(zeroOpcode.Out());
	shiftOp.Connect({ &nonzeroOpcode.Out(), &func[3], &func[4], &func[5] });
	shiftAmtOp.Connect({ &nonzeroOpcode.Out(), &func[2], &func[3], &func[4], &func[5] });
	aluBImm.Connect({ &loadOp.Out(), &storeOp.Out(), &immOp.Out() });	

	memOpByte.Connect({ &inv.Out()[0], &inv.Out()[1], &loadstore.Out() });
	memOpHalf.Connect({ &opcode[0], &inv.Out()[1], &loadstore.Out() });

	funcOpMux.Connect({ opcode, func }, zeroOpcode.Out());
	func1Inv.Connect(funcOpMux.Out()[1]);
	func2Inv.Connect(funcOpMux.Out()[2]);
	func3Inv.Connect(funcOpMux.Out()[3]);
	func4Inv.Connect(funcOpMux.Out()[4]);
	
	jumpReg.Connect({ &zeroOpcode.Out(), &func1Inv.Out(), &func2Inv.Out(), &funcOpMux.Out()[3], &func4Inv.Out() });
	jumpOp.Connect(jumpImm.Out(), jumpReg.Out());
	jumpLink.Connect(jumpOp.Out(), funcOpMux.Out()[0]);
	regWrite.Connect({ &zeroOpcode.Out(), &loadOp.Out(), &immOp.Out(), &jumpLink.Out() });
	sltop.Connect({ &funcOpMux.Out()[1], &func2Inv.Out(), &funcOpMux.Out()[3], &func4Inv.Out(), &loadstoreInv.Out() });
	mathOp.Connect({ &func2Inv.Out(), &branchOp.Out(), &loadstore.Out(), &luiOp.Out() });

	multMoveOp.Connect({ &func2Inv.Out(), &func3Inv.Out(), &funcOpMux.Out()[4]});
	multOp.Connect({ &func1Inv.Out(), &func2Inv.Out(), &funcOpMux.Out()[3], &funcOpMux.Out()[4] });
	addOr.Connect(loadstore.Out(), func1Inv.Out());
	subOr.Connect(funcOpMux.Out()[1], branchOp.Out());
	addOp.Connect(addOr.Out(), branchInv.Out());
	subOp.Connect(subOr.Out(), loadstoreInv.Out());
	jumpOrBranch.Connect(branchOp.Out(), jumpOp.Out());

	// Make LUI have an ALU opcode of A_OR_B by tweaking the 3rd bit off and 4th bit on in that case.
	Bundle<4> mathControl({ &subOp.Out(), &addOp.Out(), &luiOpInv.Out(), &luiOp.Out() });
	Bundle<4> logicControl({ &funcOpMux.Out()[0], &funcOpMux.Out()[1], &Wire::OFF, &Wire::ON });

	control.Connect({ logicControl, mathControl }, mathOp.Out());

	Bundle<4> shiftControl({ &funcOpMux.Out()[0], &funcOpMux.Out()[1], &Wire::ON, &Wire::ON });
	controlAll.Connect({ control.Out(),shiftControl }, shiftOp.Out());
}

void OpcodeDecoder::Update()
{
	inv.Update();
	halt.Update();
	zeroOpcode.Update();
	loadstore.Update();
	loadOp.Update();
	loadOpInv.Update();
	storeOp.Update();
	branchOp.Update();
	immOp.Update();
	jumpImm.Update();
	luiOp.Update();
	luiOpInv.Update();
	nonzeroOpcode.Update();
	funcOpMux.Update();
	shiftOp.Update();
	shiftAmtOp.Update();
	aluBImm.Update();
	branchInv.Update();
	loadstoreInv.Update();
	memOpByte.Update();
	memOpHalf.Update();
	func1Inv.Update();
	func2Inv.Update();
	func3Inv.Update();
	func4Inv.Update();
	jumpReg.Update();
	jumpOp.Update();
	jumpLink.Update();
	jumpOrBranch.Update();
	regWrite.Update();
	sltop.Update();
	mathOp.Update();
	multMoveOp.Update();
	multOp.Update();
	addOr.Update();
	subOr.Update();
	addOp.Update();
	subOp.Update();
	control.Update();
	controlAll.Update();
}
