#pragma once

#include "Register.h"

class InsRegister : public Register<32>
{
public:
	InsRegister()
		: Register<32>()
	{
		function = Out().Range<6>(0);
		shamt = Out().Range<5>(6);
		rdaddr = Out().Range<5>(11);
		rtaddr = Out().Range<5>(16);
		rsaddr = Out().Range<5>(21);
		opcode = Out().Range<6>(26);
		immediate = Out().Range<16>(0);
		address = Out().Range<26>(0);
	}

	const Bundle<6>& Opcode() { return opcode; }
	const Bundle<5>& RsAddr() { return rsaddr; }
	const Bundle<5>& RtAddr() { return rtaddr; }
	const Bundle<5>& RdAddr() { return rdaddr; }
	const Bundle<5>& Shamt() { return shamt; }
	const Bundle<6>& Function() { return function; }
	const Bundle<16>& Immediate() { return immediate; }
	const Bundle<26>& Address() { return address; }

private:
	Bundle<6> opcode;
	Bundle<5> rsaddr;
	Bundle<5> rtaddr;
	Bundle<5> rdaddr;
	Bundle<5> shamt;
	Bundle<6> function;
	Bundle<16> immediate;
	Bundle<26> address;
};

