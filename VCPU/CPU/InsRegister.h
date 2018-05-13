#pragma once

#include "Register.h"

class InsRegister : public Register<32>
{
public:
	InsRegister();
	const Bundle<6>& Opcode() { return opcode; }
	const Bundle<5>& RsAddr() { return rsaddr; }
	const Bundle<5>& RtAddr() { return rdaddr; }
	const Bundle<5>& RdAddr() { return rtaddr; }
	const Bundle<5>& Shamt() { return shamt; }
	const Bundle<6>& Function() { return function; }
	const Bundle<16>& Immediate() { return immediate; }
	const Bundle<26>& Address() { return address; }

private:
	Bundle<6> opcode;
	Bundle<5> rsaddr;
	Bundle<5> rdaddr;
	Bundle<5> rtaddr;
	Bundle<5> shamt;
	Bundle<6> function;
	Bundle<16> immediate;
	Bundle<26> address;
};

InsRegister::InsRegister()
	: Register<32>()
{
	function = Out().Range<0, 6>();
	shamt = Out().Range<6, 11>();
	rtaddr = Out().Range<11, 16>();
	rdaddr = Out().Range<16, 21>();
	rsaddr = Out().Range<21, 26>();
	opcode = Out().Range<26, 32>();
	immediate = Out().Range<0, 16>();
	address = Out().Range<0, 26>();
}