#include "OpcodeDecoder.h"

OpcodeDecoder::OpcodeDecoder()
{
	out.Connect({ &branchOp.Out(), &loadOp.Out(), &storeOp.Out(), &zeroOpcode.Out(),
		&aluBImm.Out(), &regWrite.Out(), &sltop.Out(), &shiftOp.Out(),&shiftAmtOp.Out(),
		&halt.Out(), &jumpOp.Out(), &jumpLink.Out(), &jumpReg.Out(),
		&luiOp.Out(), &mathOp.Out(), &funcOpMux.Out()[0], &funcOpMux.Out()[1],
		&opcodeIn.Out()[0], &opcodeIn.Out()[1], &opcodeIn.Out()[2]
		});
}

void OpcodeDecoder::Connect(const Bundle<6>& opcode, const Bundle<6>& func)
{
	inv.Connect(opcode);
	opcodeIn.Connect(inv.Out()); // This is stupid but a simple way to store the actual opcode.
	halt.Connect(opcode);
	zeroOpcode.Connect(inv.Out());
	loadstore.Connect(inv.Out()[4], opcode[5]);
	loadstoreInv.Connect(loadstore.Out());
	loadOp.Connect(inv.Out()[3], loadstore.Out());
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

	funcOpMux.Connect({ opcode, func }, zeroOpcode.Out());
	func1Inv.Connect(funcOpMux.Out()[1]);
	func2Inv.Connect(funcOpMux.Out()[2]);
	func4Inv.Connect(funcOpMux.Out()[4]);
	
	jumpReg.Connect({ &zeroOpcode.Out(), &func1Inv.Out(), &func2Inv.Out(), &funcOpMux.Out()[3], &func4Inv.Out() });
	jumpOp.Connect(jumpImm.Out(), jumpReg.Out());
	jumpLink.Connect(jumpOp.Out(), funcOpMux.Out()[0]);
	regWrite.Connect({ &zeroOpcode.Out(), &loadOp.Out(), &immOp.Out(), &jumpLink.Out() });
	sltop.Connect({ &funcOpMux.Out()[1], &func2Inv.Out(), &funcOpMux.Out()[3], &func4Inv.Out(), &loadstoreInv.Out() });
	mathOp.Connect({ &func2Inv.Out(), &branchOp.Out(), &loadstore.Out(), &luiOp.Out() });
	addOr.Connect(loadstore.Out(), func1Inv.Out());
	subOr.Connect(funcOpMux.Out()[1], branchOp.Out());
	addOp.Connect(addOr.Out(), branchInv.Out());
	subOp.Connect(subOr.Out(), loadstoreInv.Out());

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
	opcodeIn.Update();
	halt.Update();
	zeroOpcode.Update();
	loadstore.Update();
	loadOp.Update();
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
	func1Inv.Update();
	func2Inv.Update();
	func4Inv.Update();
	jumpReg.Update();
	jumpOp.Update();
	jumpLink.Update();
	regWrite.Update();
	sltop.Update();
	mathOp.Update();
	addOr.Update();
	subOr.Update();
	addOp.Update();
	subOp.Update();
	control.Update();
	controlAll.Update();
}
