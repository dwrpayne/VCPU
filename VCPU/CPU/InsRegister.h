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

	const Bundle<6>& Opcode() const { return opcode; }
	const Bundle<5>& RsAddr() const { return rsaddr; }
	const Bundle<5>& RtAddr()  const { return rtaddr; }
	const Bundle<5>& RdAddr()  const { return rdaddr; }
	const Bundle<5>& Shamt()  const { return shamt; }
	const Bundle<6>& Function()  const { return function; }
	const Bundle<16>& Immediate()  const { return immediate; }
	const Bundle<26>& Address()  const { return address; }

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

