#include "OpcodeDecoder.h"

void OpcodeDecoder::Connect(const Bundle<6>& opcode)
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
}
