#pragma once

#include "Component.h"
#include "Register.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"

class BufferIFID : public Component
{
public:
	void Connect(const Bundle<32>& instruction, const Bundle<32>& pcInc)
	{
		IR.Connect(instruction, Wire::ON);
		PCinc.Connect(pcInc, Wire::ON);
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
	void Connect(const Bundle<5>& rt, const Bundle<5>& rd, const Bundle<32>& signExtImm, const Bundle<32>& regR1, const Bundle<32>& regR2, const Bundle<32> pcInc, const Bundle<6>& op, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec, const Bundle<4> alucontrol)
	{
		RT.Connect(rt, Wire::ON);
		RD.Connect(rd, Wire::ON);
		signExt.Connect(signExtImm, Wire::ON);
		reg1.Connect(regR1, Wire::ON);
		reg2.Connect(regR2, Wire::ON);
		PCinc.Connect(pcInc, Wire::ON);
		opcode.Connect(op, Wire::ON);
		opcodeControl.Connect(opcodeDec, Wire::ON);
		aluControl.Connect(alucontrol, Wire::ON);
	}
	void Update()
	{
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

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

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
	void Connect(const Bundle<5>& rwrite, const Bundle<32>& regR2, const Bundle<32>& aluout, const Bundle<32>& pcjumpAdd, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, Wire::ON);
		reg2.Connect(regR2, Wire::ON);
		aluOut.Connect(aluout, Wire::ON);
		pcJumpAddr.Connect(pcjumpAdd, Wire::ON);
		opcodeControl.Connect(opcodeDec, Wire::ON);
	}
	void Update()
	{
		Rwrite.Update();
		reg2.Update();
		aluOut.Update();
		pcJumpAddr.Update();
		opcodeControl.Update();
	}
	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> Rwrite;
	Register<32> reg2;
	Register<32> aluOut;
	Register<32> pcJumpAddr;
	Register<OpcodeDecoder::OUT_WIDTH> opcodeControl;
};

class BufferMEMWB : public Component
{
public:
	void Connect(const Bundle<5>& rwrite, const Bundle<32>& aluout, const Bundle<32>& memout, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		Rwrite.Connect(rwrite, Wire::ON);
		aluOut.Connect(aluout, Wire::ON);
		memOut.Connect(memout, Wire::ON);
		opcodeControl.Connect(opcodeDec, Wire::ON);
	}
	void Update()
	{
		Rwrite.Update();
		aluOut.Update();
		memOut.Update();
		opcodeControl.Update();
	}
	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> Rwrite;
	Register<32> aluOut;
	Register<32> memOut;
	Register<OpcodeDecoder::OUT_WIDTH> opcodeControl;
};
