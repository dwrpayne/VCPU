#include "ALUControl.h"

void ALUControl::Connect(const Wire & loadstore, const Wire & branch, const Wire& immediate, const Wire & rformat, const Bundle<6>& opcode, const Bundle<6>& func)
{
	func1Inv.Connect(func[1]);
	logicOp.Connect(func[2], immediate);
	logicMux.Connect({ func.Range<0,2>(), opcode.Range<0,2>() }, immediate);

	addOp.Connect(loadstore, func1Inv.Out());
	subOp.Connect(func[1], branch);
	
	control.Connect({ Bundle<2>({ &subOp.Out(), &addOp.Out() }), logicMux.Out() }, logicOp.Out());

	out = { &control.Out()[0], &control.Out()[1], &mathOp.Out(), &logicOp.Out() };
}

void ALUControl::Update()
{
}
