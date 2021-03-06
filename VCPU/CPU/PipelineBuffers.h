#pragma once

#include "Component.h"
#include "Register.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "ALU.h"

class BufferIFID : public Component
{
public:
	void Connect(const Wire& enable, const Bundle<32>& instruction, const Bundle<32>& pcInc)
	{
		IR.Connect(instruction, enable);
		PCinc.Connect(pcInc, enable);
	}
	void Update()
	{
		IR.Update();
		PCinc.Update();
	}

	InsRegister IR;
	Register<32> PCinc;
};

class BufferIDEX : public Component
{
public:
	void Connect(const Wire& enable, const Wire& flush, const Bundle<5>& rs, const Bundle<5>& rt, const Bundle<5>& rd, const Bundle<32>& immext, const Bundle<32>& regR1, const Bundle<32>& regR2,
		const Bundle<32>& jumpaddr, const Bundle<32>& pcInc, const Bundle<6>& op, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec, const Bundle<4> alucontrol, const Wire& branchtaken)
	{
		RS.Connect(rs, enable, flush);
		RT.Connect(rt, enable, flush);
		RD.Connect(rd, enable, flush);
		pcJumpAddr.Connect(jumpaddr, enable, flush);
		immExt.Connect(immext, enable, flush);
		reg1.Connect(regR1, enable, flush);
		reg2.Connect(regR2, enable, flush);
		PCinc.Connect(pcInc, enable, flush);
		opcode.Connect(op, enable, flush);
		opcodeControl.Connect(opcodeDec, enable, flush);
		aluControl.Connect(alucontrol, enable, flush);
		branchTaken.Connect(Bundle<1>(branchtaken), enable, flush);
	}
	void Update()
	{
		RS.Update();
		RT.Update();
		RD.Update();
		pcJumpAddr.Update();
		immExt.Update();
		reg1.Update();
		reg2.Update();
		PCinc.Update();
		opcode.Update();
		opcodeControl.Update();
		aluControl.Update();
		branchTaken.Update();
	}

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() const { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	RegisterReset<5> RS;
	RegisterReset<5> RT;
	RegisterReset<5> RD;
	RegisterReset<32> pcJumpAddr;
	RegisterReset<32> immExt;
	RegisterReset<32> reg1;
	RegisterReset<32> reg2;
	RegisterReset<32> PCinc;
	RegisterReset<6> opcode;
	RegisterReset<OpcodeDecoder::OUT_WIDTH> opcodeControl;
	RegisterReset<4> aluControl;
	RegisterReset<1> branchTaken;
};

class BufferEXMEM : public Component
{
public:
	void Connect(const Wire& enable, const Wire& flush, const Bundle<5>& rwrite, const Bundle<32>& regR2, const Bundle<32>& aluout, const ALU<32>::ALUFlags& flags,
		const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		// Only flush if we aren't stalling!
		flushAnd.Connect(enable, flush);

		Rwrite.Connect(rwrite, enable, flushAnd.Out());
		reg2.Connect(regR2, enable, flushAnd.Out());
		aluOut.Connect(aluout, enable, flushAnd.Out());
		aluFlags.Connect(flags, enable, flushAnd.Out());
		opcodeControl.Connect(opcodeDec, enable, flushAnd.Out());
	}
	void Update()
	{
		flushAnd.Update();
		Rwrite.Update();
		reg2.Update();
		aluOut.Update();
		aluFlags.Update();
		opcodeControl.Update();
	}
	
	ALU<32>::ALUFlags Flags() const { return ALU<32>::ALUFlags(aluFlags.Out()); }

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() const { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	RegisterReset<5> Rwrite;
	RegisterReset<32> reg2;
	RegisterReset<32> aluOut;
	RegisterReset<4> aluFlags;
	RegisterReset<OpcodeDecoder::OUT_WIDTH> opcodeControl;

private:
	// Flushing EXMEM leads the cache to behave badly if it was in the middle of a pending operation.
	// It relies on this pipeline buffer to maintain its state for it. Maybe that should be rethought.
	AndGate flushAnd;
};

class BufferMEMWB : public Component
{
public:
	void Connect(const Wire& enable, const Bundle<5>& rwrite, const Bundle<32>& rwritedata, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, enable);
		RWriteData.Connect(rwritedata, enable);
		opcodeControl.Connect(opcodeDec, enable);
	}
	void Update()
	{
		Rwrite.Update();
		RWriteData.Update();
		opcodeControl.Update();
	}

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() const { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> Rwrite;
	Register<32> RWriteData;
	Register<OpcodeDecoder::OUT_WIDTH> opcodeControl;
};
