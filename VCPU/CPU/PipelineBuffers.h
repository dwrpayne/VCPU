#pragma once

#include "Component.h"
#include "Register.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "ALU.h"

class BufferIFID : public Component
{
public:
	void Connect(const Wire& enable, const Bundle<32>& instruction, const Bundle<32>& pcInc, const Wire& flush)
	{
		IR.Connect(instruction, enable, flush);
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
	void Connect(const Wire& enable, const Bundle<5>& rwrite, const Bundle<32>& regR2, const Bundle<32>& aluout, const ALU<32>::ALUFlags& flags, 
		const Bundle<32>& pcjumpAdd, const Wire& branchtaken, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, enable);
		reg2.Connect(regR2, enable);
		aluOut.Connect(aluout, enable);
		aluFlags.Connect(flags, enable);
		pcJumpAddr.Connect(pcjumpAdd, enable);
		branchTaken.Connect(Bundle<1>(branchtaken), enable);
		opcodeControl.Connect(opcodeDec, enable);
	}
	void Update()
	{
		Rwrite.Update();
		reg2.Update();
		aluOut.Update();
		aluFlags.Update();
		pcJumpAddr.Update();
		branchTaken.Update();
		opcodeControl.Update();
	}
	
	ALU<32>::ALUFlags Flags() const { return ALU<32>::ALUFlags(aluFlags.Out()); }

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() const { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> Rwrite;
	Register<32> reg2;
	Register<32> aluOut;
	Register<4> aluFlags;
	Register<32> pcJumpAddr;
	Register<1> branchTaken;
	Register<OpcodeDecoder::OUT_WIDTH> opcodeControl;
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
