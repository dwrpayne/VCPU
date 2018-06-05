#pragma once

#include "Component.h"
#include "Register.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"

class BufferIFID : public Component
{
public:
	void Connect(const Wire& go, const Bundle<32>& instruction, const Bundle<32>& pcInc)
	{
		IR.Connect(instruction, go);
		PCinc.Connect(pcInc, go);
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
	void Connect(const Wire& go, const Bundle<5>& rs, const Bundle<5>& rt, const Bundle<5>& rd, const Bundle<32>& signExtImm, const Bundle<32>& regR1, const Bundle<32>& regR2, const Bundle<32> pcInc, const Bundle<6>& op, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec, const Bundle<4> alucontrol)
	{
		RS.Connect(rs, go);
		RT.Connect(rt, go);
		RD.Connect(rd, go);
		signExt.Connect(signExtImm, go);
		reg1.Connect(regR1, go);
		reg2.Connect(regR2, go);
		PCinc.Connect(pcInc, go);
		opcode.Connect(op, go);
		opcodeControl.Connect(opcodeDec, go);
		aluControl.Connect(alucontrol, go);
	}
	void Update()
	{
		RS.Update();
		RT.Update();
		RD.Update();
		signExt.Update();
		reg1.Update();
		reg2.Update();
		PCinc.Update();
		opcode.Update();
		opcodeControl.Update();
		aluControl.Update();
	}

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() const { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> RS;
	Register<5> RT;
	Register<5> RD;
	Register<32> signExt;
	Register<32> reg1;
	Register<32> reg2;
	Register<32> PCinc;
	Register<6> opcode;
	Register<OpcodeDecoder::OUT_WIDTH> opcodeControl;
	Register<4> aluControl;

};

class BufferEXMEM : public Component
{
public:
	void Connect(const Wire& go, const Bundle<5>& rwrite, const Bundle<32>& regR2, const Bundle<32>& aluout, const ALU<32>::ALUFlags& flags, const Bundle<32>& pcjumpAdd, const Wire& branchtaken, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, go);
		reg2.Connect(regR2, go);
		aluOut.Connect(aluout, go);
		aluFlags.Connect(flags, go);
		pcJumpAddr.Connect(pcjumpAdd, go);
		branchTaken.Connect(Bundle<1>(branchtaken), go);
		opcodeControl.Connect(opcodeDec, go);
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
	void Connect(const Wire& go, const Bundle<5>& rwrite, const Bundle<32>& rwritedata, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, go);
		RWriteData.Connect(rwritedata, go);
		opcodeControl.Connect(opcodeDec, go);
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
