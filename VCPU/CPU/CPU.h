#pragma once

#include "Component.h"
#include "Register.h"
#include "RegisterFile.h"
#include "Memory.h"
#include "ALU.h"
#include "MuxBundle.h"

#include "OpcodeDecoder.h"
#include "InsRegister.h"
#include "ALUControl.h"

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
	void Connect(const Bundle<5>& rt, const Bundle<5>& rd, const Bundle<32>& signExtImm, const Bundle<32>& regR1, const Bundle<32>& regR2, const Bundle<32> pcInc, const Bundle<6>& op, const OpcodeDecoder::OpcodeDecoderBundle& opcodeDec)
	{
		RT.Connect(rt, Wire::ON);
		RD.Connect(rd, Wire::ON);
		signExt.Connect(signExtImm, Wire::ON);
		reg1.Connect(regR1, Wire::ON);
		reg2.Connect(regR2, Wire::ON);
		PCinc.Connect(pcInc, Wire::ON);
		opcode.Connect(op, Wire::ON);
		opcodeControl.Connect(opcodeDec, Wire::ON);
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
	}

	OpcodeDecoder::OpcodeDecoderBundle OpcodeControl() { return OpcodeDecoder::OpcodeDecoderBundle(opcodeControl.Out()); }

	Register<5> RT;
	Register<5> RD;
	Register<32> signExt;
	Register<32> reg1;
	Register<32> reg2;
	Register<32> PCinc;
	Register<6> opcode;
	Register<8> opcodeControl;

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
	Register<8> opcodeControl;
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
	Register<8> opcodeControl;
};

class CPU : public Component
{
public:
	void Connect();
	void Update();

	int cycles;

	typedef Memory<32, 128> InsMemory;
	typedef Memory<32, 128> MainMemory; // Todo: cache levels
	typedef RegisterFile<32, 32> RegFile;

	void ConnectToLoader(Bundle<32>& addr, Bundle<32> ins);
	void LoadInstruction();

private:
	MuxBundle<32, 2> pcInMux;
	Register<32> pc;
	FullAdderN<32> pcIncrementer;
		
	InsMemory instructionMem;
	BufferIFID bufIFID;
	
	OpcodeDecoder opcodeControl;
	RegFile regFile;

	BufferIDEX bufIDEX;

	ALUControl aluControl;
	FullAdderN<32> pcJumpAdder;
	MuxBundle<32, 2> aluBInputMux;
	ALU<32> alu;
	MuxBundle<RegFile::ADDR_BITS, 2> regFileWriteAddrMux;

	BufferEXMEM bufEXMEM;

	MainMemory mainMem;

	BufferMEMWB bufMEMWB;

	MuxBundle<32, 2> regWriteDataMux;
};