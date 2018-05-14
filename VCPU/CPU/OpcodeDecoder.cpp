#include "OpcodeDecoder.h"

void OpcodeDecoder::Connect(const Bundle<6>& opcode)
{
	inv.Connect(opcode);
	rFormat.Connect(inv.Out());
	lw.Connect({ &opcode[0], &opcode[1], &inv.Out()[2], &inv.Out()[3], &inv.Out()[4], &opcode[5] });
	sw.Connect({ &opcode[0], &opcode[1], &inv.Out()[2], &opcode[3], &inv.Out()[4], &opcode[5] });
	beq.Connect({ &inv.Out()[0], &inv.Out()[1], &opcode[2], &inv.Out()[3], &inv.Out()[4], &inv.Out()[5] });
	aluSrc.Connect(lw.Out(), sw.Out());
	regWrite.Connect(rFormat.Out(), lw.Out());
}

void OpcodeDecoder::Update()
{
	inv.Update();
	rFormat.Update();
	lw.Update();
	sw.Update();
	beq.Update();
	aluSrc.Update();
	regWrite.Update();
}
