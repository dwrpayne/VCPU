#include "ALUControl.h"

void ALUControl::Connect(const Wire & loadstore, const Wire & branch, const Wire& immediate, const Wire & rformat, const Bundle<6>& opcode, const Bundle<6>& func)
{
	zeroOpcode.Connect(opcode);
	funcOpMux.Connect({ opcode.Range<0,3>(), func.Range<0,3>() }, zeroOpcode.Out());

	branchInv.Connect(branch);
	loadstoreInv.Connect(loadstore);
		
	func1Inv.Connect(funcOpMux.Out()[1]);
	func2Inv.Connect(funcOpMux.Out()[2]);
	mathOp.Connect({ &func2Inv.Out(), &branch, &loadstore });
	addOr.Connect(loadstore, func1Inv.Out());
	subOr.Connect(funcOpMux.Out()[1], branch);
	addOp.Connect(addOr.Out(), branchInv.Out());
	subOp.Connect(subOr.Out(), loadstoreInv.Out());

	Bundle<4> mathControl({ &subOp.Out(), &addOp.Out(), &Wire::ON, &Wire::OFF });
	Bundle<4> logicControl({ &funcOpMux.Out()[0], &funcOpMux.Out()[1], &Wire::OFF, &Wire::ON});
	
	control.Connect({logicControl, mathControl}, mathOp.Out());
}

void ALUControl::Update()
{
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
