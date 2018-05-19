#pragma once

#include "Component.h"
#include "Bundle.h"
#include "Adder.h"
#include "Inverter.h"
#include "MuxBundle.h"
#include "MultiGate.h"
#include "AndGate.h"
#include "OrGate.h"
#include "NorGate.h"
#include "XorGate.h"

enum ALU_OPCODE : uint8_t
{
	A,
	A_PLUS_ONE,
	A_MINUS_ONE,
	A_ALSO,
	A_MINUS_B_MINUS_ONE,
	A_MINUS_B,
	A_PLUS_B,
	A_PLUS_B_PLUS_ONE,
	A_AND_B,
	A_OR_B,
	A_XOR_B,
	A_NOR_B,
	A_SHR,
	A_SHL,
	UNUSED,
	UNUSED2,
	MAX
};

/*******************************
      VALU opcode table
	  C3 C2 C1 C0	Output
	  -----------------
	  (C3==0: arithmetic)
	   0  0  0  0	A
	   0  0  0  1	A + 1
	   0  0  1  0	A - 1
	   0  0  1  1	A
	   0  1  0  0	A - B - 1
	   0  1  0  1	A - B
	   0  1  1  0	A + B
	   0  1  1  1	A + B + 1

	   (C3==1, C2==0: logic)
	   1  0  0  0	A AND B
	   1  0  0  1	A OR B
	   1  0  1  0	A XOR B
	   1  0  1  1	A NOR B

	   (C3==1, C2==1: shifts)
	   1  1  0  0	SHR A
	   1  1  0  1	SHL A
	   1  1  1  0	
	   1  1  1  1	

C[2:0] are passed through to the Adder as those are its opcodes

*******************************/

template <unsigned int N>
class ALU : public Component
{
public:
	ALU();
	void Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<4>& control);
	void Update();

	const Bundle<N>& Out() { return outMux.Out(); }

	class ALUFlags : public Bundle<4>
	{
	public:
		ALUFlags()
			: Bundle<4>()
		{}
		ALUFlags(std::initializer_list<const Wire*> list)
			: Bundle<4>(list)
		{}
		ALUFlags(const Bundle<4>& other)
			: Bundle<4>(other)
		{}
		const Wire& Zero() const { return Get(0); }
		const Wire& Carry() const { return Get(1); }  // TODO: When subtracting, seems to return true when a >= b? Weird..
		const Wire& Overflow() const { return Get(2); }
		const Wire& Negative() const { return Get(3); }

	};
	const ALUFlags& Flags() { return flags; }


private:
	Adder<N> adder;
	MultiGate<AndGate, N> ands;
	MultiGate<OrGate, N> ors;
	MultiGate<XorGate, N> xors;
	InverterN<N> invs;
	MuxBundle<N, 8> logicShiftMux;
	MuxBundle<N, 2> outMux;
	NorGateN<N> zeroOut;
	ALUFlags flags;
};

template<unsigned int N>
inline ALU<N>::ALU()
{
	flags.Connect({ &zeroOut.Out(), &adder.Cout(), &adder.Overflow(), &Out()[N - 1] });
}

template<unsigned int N>
inline void ALU<N>::Connect(const Bundle<N>& a, const Bundle<N>& b, const Bundle<4>& control)
{
	adder.Connect(a, b, control.Range<0, 3>());
	
	// Logic
	ands.Connect(a, b);
	ors.Connect(a, b);
	xors.Connect(a, b);
	invs.Connect(ors.Out());

	// Shift
	Bundle<N> shiftL, shiftR;
	shiftR.Connect(0, a.Range<1, N>());
	shiftR.Connect(N - 1, Wire::OFF);
	shiftL.Connect(0, Wire::OFF);
	shiftL.Connect(1, a.Range<0, N - 1>());

	logicShiftMux.Connect({ ands.Out(), ors.Out(), xors.Out(), invs.Out(), shiftR, shiftL, Bundle<N>::OFF, Bundle<N>::OFF }, control.Range<0, 3>());
	outMux.Connect({ adder.Out(), logicShiftMux.Out() }, control[3]);

	zeroOut.Connect(outMux.Out());
}

template<unsigned int N>
inline void ALU<N>::Update()
{
	adder.Update();
	ands.Update();
	ors.Update();
	xors.Update();
	invs.Update();
	logicShiftMux.Update();
	outMux.Update();
	zeroOut.Update();
}
