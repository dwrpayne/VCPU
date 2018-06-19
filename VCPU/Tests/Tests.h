#pragma once
#include <iostream>
#include <cmath>
#include <map>
#include "TestHelpers.h"
#include "Tools/MagicBundle.h"
#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "MultiGate.h"
#include "XorGate.h"
#include "XNorGate.h"
#include "Inverter.h"
#include "SRLatch.h"
#include "JKFlipFlop.h"
#include "DFlipFlop.h"
#include "DFlipFlopReset.h"
#include "Bundle.h"
#include "Register.h"
#include "Counter.h"
#include "FullAdder.h"
#include "Adder.h"
#include "Multiplexer.h"
#include "Encoder.h"
#include "Matcher.h"
#include "Shifter.h"
#include "Masker.h"
#include "Comparator.h"
#include "MuxBundle.h"
#include "SelectBundle.h"
#include "Decoder.h"
#include "ALU.h"
#include "RegisterFile.h"
#include "CacheLine.h"
#include "Multiplier.h"
#include "CircularBuffer.h"
#include "RequestBuffer.h"

#ifdef DEBUG
bool TestAndGate(const Wire& a, const Wire& b)
{
	AndGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() == (a.On() && b.On());
}

bool TestOrGate(const Wire& a, const Wire& b)
{
	OrGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() == (a.On() || b.On());
}

bool TestNandGate(const Wire& a, const Wire& b)
{
	NandGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() != (a.On() && b.On());
}

bool TestNorGate(const Wire& a, const Wire& b)
{
	NorGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() != (a.On() || b.On());
}

bool TestXorGate(const Wire& a, const Wire& b)
{
	XorGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() == (a.On() ^ b.On());
}

bool TestXNorGate(const Wire& a, const Wire& b)
{
	XNorGate test;
	test.Connect(a, b);
	test.Update();
	return test.Out().On() != (a.On() ^ b.On());
}

bool TestAndGate4(const Bundle<4>& b)
{
	AndGateN<4> test;
	test.Connect(b);
	test.Update();
	return test.Out().On() == (b[0].On() && b[1].On() && b[2].On() && b[3].On());
}

bool TestOrGate4(const Bundle<4>& b)
{
	OrGateN<4> test;
	test.Connect(b);
	test.Update();
	return test.Out().On() == (b[0].On() || b[1].On() || b[2].On() || b[3].On());
}

bool TestMultiGate(Verbosity verbosity)
{
	bool success = true;
	int i = 0;
	MultiGate<XorGate,8> test;
	MagicBundle<8> a_reg;
	MagicBundle<8> b_reg;
	test.Connect(a_reg, b_reg);
	for (const auto&[a, b] : std::map<int, int>({ { -64, -64 },{ 0, 0 },{ 11, 116 },{ 4, -121 } }))
	{
		a_reg.Write(a);
		b_reg.Write(b);
		test.Update();
		success &= TestState(i++, a ^ b, test.Out().Read(), verbosity);
	}

	return success;
}

bool TestMultiGateN(Verbosity verbosity)
{
	bool success = true;
	int i = 0;
	MultiGateN<AndGateN<4>, 4, 4> test;
	MagicBundle<4> r1, r2, r3, r4;
	test.Connect({ r1, r2, r3, r4 });
	for (const auto&[a, b, c, d] : std::vector<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>>({ { 15U, 2U, 0, 15U },
		{ 0, 0, 0, 0 },
		{ 15U, 15U, 15U, 15U},
		}))
	{
		r1.Write(a);
		r2.Write(b);
		r3.Write(c);
		r4.Write(d);
		test.Update();
		unsigned int val = ((a == 15) << 3) + ((b == 15) << 2) + ((c == 15) << 1) + (d == 15);
		success &= TestState(i++, val, test.Out().UnsignedRead(), verbosity);
	}

	return success;
}

bool TestMultiGateNBitwise(Verbosity verbosity)
{
	bool success = true;
	int i = 0;
	MultiGateNBitwise<OrGateN<4>,32,4> test;
	MagicBundle<32> r1, r2, r3, r4;
	test.Connect({ r1, r2, r3, r4 });
	for (const auto&[a, b, c, d] : std::vector<std::tuple<int, int, int, int>>({ { -123545, 97869,4524975, 9237456 },
																				{ 123456789, 1048576,4524975, 1123123 },
																				{ 0, 0,0, 1 },
		}))
	{
		r1.Write(a);
		r2.Write(b);
		r3.Write(c);
		r4.Write(d);
		test.Update();
		success &= TestState(i++, (unsigned int)(a | b | c | d), test.Out().UnsignedRead(), verbosity);
	}

	return success;
}

bool TestFullAdder(const Wire& a, const Wire& b, const Wire& c)
{
	FullAdder test;
	test.Connect(a, b, c);
	test.Update();
	bool success = ((int)test.S().On() == ((int)a.On() + (int)b.On() + (int)c.On()) % 2);
	success &= (test.Cout().On() == ((int)a.On() + (int)b.On() + (int)c.On()) >= 2);
	return success;
}

bool TestInverter(const Wire& a)
{
	Inverter test;
	test.Connect(a);
	test.Update();
	bool out = test.Out().On();
	return out != a.On();
}

bool TestInverter3(const Wire& a, const Wire& b, const Wire& c)
{
	InverterN<3> test;
	test.Connect({ &a, &b, &c });
	test.Update();
	return (test.Out()[0].On() != a.On()) &&
		   (test.Out()[1].On() != b.On()) &&
		   (test.Out()[2].On() != c.On());
}

bool TestSRLatch(Verbosity verbosity)
{
	SRLatch test;
	test.Connect(Wire::OFF, Wire::ON); 
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);
	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	return success;
}

bool TestJKFlipFlop(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	JKFlipFlop test;
	test.Connect(Wire::OFF, Wire::OFF);
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	test.Update();
	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);
	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	return success;
}

bool TestDFlipFlop(Verbosity verbosity)
{
	DFlipFlop test;
	test.Connect(Wire::OFF, Wire::ON);
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	return success;
}

bool TestDFlipFlopReset(Verbosity verbosity)
{
	DFlipFlopReset test;
	test.Connect(Wire::OFF, Wire::ON, Wire::OFF);
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::OFF, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::ON, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::ON, Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	test.Connect(Wire::ON, Wire::ON, Wire::OFF); test.Update();
	success &= TestState(i++, q.On(), true, verbosity);
	success &= TestState(i++, notq.On(), false, verbosity);

	test.Connect(Wire::OFF, Wire::OFF, Wire::ON); test.Update();
	success &= TestState(i++, q.On(), false, verbosity);
	success &= TestState(i++, notq.On(), true, verbosity);

	return success;
}

bool TestBundle(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Bundle<4> test1({ &Wire::ON, &Wire::OFF, &Wire::OFF, &Wire::OFF });
	success &= TestState(i++, 1, test1.Read(), verbosity);

	Bundle<4> test2({ &Wire::ON, &Wire::OFF, &Wire::ON, &Wire::OFF });
	success &= TestState(i++, 5, test2.Read(), verbosity);

	Bundle<4> test3({ &Wire::ON, &Wire::ON, &Wire::ON, &Wire::ON });
	success &= TestState(i++, -1, test3.Read(), verbosity);

	Bundle<4> test4({ &Wire::ON, &Wire::ON, &Wire::OFF, &Wire::ON });
	success &= TestState(i++, -5, test4.Read(), verbosity);
	
	success &= TestState(i++, 0, Bundle<8>::OFF.Read(), verbosity);
	success &= TestState(i++, 255U, Bundle<8>::ON.UnsignedRead(), verbosity);
	success &= TestState(i++, 0xaaU, Bundle<8>::ERROR.UnsignedRead(), verbosity);
	success &= TestState(i++, 0xdeadU, Bundle<16>::ERROR.UnsignedRead(), verbosity);
	success &= TestState(i++, 0xdeadbeefU, Bundle<32>::ERROR.UnsignedRead(), verbosity);
	
	Bundle<8> test(121);
	Bundle<16> testExt = test.ZeroExtend<16>();
	success &= TestState(i++, 121, testExt.Read(), verbosity);
	success &= TestState(i++, 121, testExt.Range<8>().Read(), verbosity);
	success &= TestState(i++, 60, testExt.Range<8>(1).Read(), verbosity);
	success &= TestState(i++, 242, test.ShiftZeroExtend<16>(1).Read(), verbosity);
	success &= TestState(i++, 242U, test.ShiftZeroExtendCanLose<8>(1).UnsignedRead(), verbosity);
	success &= TestState(i++, 228U, test.ShiftZeroExtendCanLose<8>(2).UnsignedRead(), verbosity);
	success &= TestState(i++, 60, testExt.Range<8>(1).Read(), verbosity);

	Bundle<8> testneg(-121);
	Bundle<16> testnegExt = testneg.SignExtend<16>();
	success &= TestState(i++, -121, testneg.SignExtend<16>().Read(), verbosity);
	success &= TestState(i++, 256-121, testneg.ZeroExtend<16>().Read(), verbosity);
	success &= TestState(i++, -61, testneg.ShiftRightSignExtend<8>(1).Read(), verbosity);
	success &= TestState(i++, -16, testneg.ShiftRightSignExtend<8>(3).Read(), verbosity);
	success &= TestState(i++, -16, testneg.ShiftRightSignExtend<16>(3).Read(), verbosity);
	
	success &= TestState(i++, 14, Bundle<8>(14).Read(), verbosity);
	success &= TestState(i++, -111, Bundle<8>(-111).Read(), verbosity);


	return success;
}

bool TestRegister(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Register<8> reg;
	MagicBundle<8> data;
	Wire load(true);
	reg.Connect(data, load);
	data.Write(0);
	reg.Update();
	int prevval = 0;
	for (int val : { -128, -92, 0, 1, 77, 127 })
	{
		data.Write(val);
		load.Set(false);
		reg.Update();
		success &= TestState(i++, prevval, reg.Out().Read(), verbosity);

		load.Set(true);
		reg.Update();
		success &= TestState(i++, val, reg.Out().Read(), verbosity);
		prevval = val;
	}

	load.Set(true);
	data.Write(128);
	reg.Update();
	success &= TestState(i++, 0, reg.Out().Read(), verbosity);

	return success;
}

bool TestCounter(Verbosity verbosity)
{
	bool success = true;
	int i = 0;
	const int bits = 5;
	Counter<bits> test;
	Wire clear(false);
	Wire enable(false);
	test.Connect(clear, enable);
	test.Update();
	enable.Set(true);

	for (unsigned int cycle = 1; cycle < 50; ++cycle)
	{
		test.Update();
		success &= TestState(i++, cycle%pow2(bits), test.Out().UnsignedRead(), verbosity);
	}
	enable.Set(false);
	for (unsigned int cycle = 0; cycle < 5; ++cycle)
	{
		test.Update();
		success &= TestState(i++, 17U, test.Out().UnsignedRead(), verbosity);
	}
	clear.Set(true);
	for (unsigned int cycle = 0; cycle < 5; ++cycle)
	{
		test.Update();
		success &= TestState(i++, 0U, test.Out().UnsignedRead(), verbosity);
	}
	enable.Set(true);
	for (unsigned int cycle = 0; cycle < 5; ++cycle)
	{
		test.Update();
		success &= TestState(i++, 0U, test.Out().UnsignedRead(), verbosity);
	}
	clear.Set(false);
	for (unsigned int cycle = 1; cycle < 50; ++cycle)
	{
		test.Update();
		success &= TestState(i++, (cycle)%pow2(bits), test.Out().UnsignedRead(), verbosity);
	}

	return success;
}

bool TestFreqSwitcher(Verbosity verbosity)
{
	bool success = true;
	int i = 0;
	ClockFreqSwitcher<8> test;
	test.Connect();
	for (int cycle = 1; cycle < 50; cycle++)
	{
		test.Update();
		success &= TestState(i++, !(cycle % 8), test.Pulse().On(), verbosity);
		success &= TestState(i++, (bool)(cycle % 8), test.NotPulse().On(), verbosity);
	}
	return success;
}

bool TestMultiplexer2(const Wire& a)
{
	Multiplexer<2> test;
	test.Connect({ &Wire::ON, &Wire::OFF }, a);
	test.Update();
	return test.Out().On() != a.On();	
}

bool TestMuxBundle(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	MuxBundle<32, 2> test;
	Wire sel;
	test.Connect({ MagicBundle<32>(12345), MagicBundle<32>(9876) }, sel);

	test.Update();
	success &= TestState(i++, 12345, test.Out().Read(), verbosity);

	sel.Set(true);
	test.Update();
	success &= TestState(i++, 9876, test.Out().Read(), verbosity);
	return success;
}

bool TestSelectBundle(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	SelectBundle<32, 4> test;
	MagicBundle<4> sel;

	test.Connect({ MagicBundle<32>(12345), MagicBundle<32>(9876), MagicBundle<32>(3333333), MagicBundle<32>(4444444) }, sel);
	sel.Write(1);
	test.Update();
	success &= TestState(i++, 12345, test.Out().Read(), verbosity);

	sel.Write(2);
	test.Update();
	success &= TestState(i++, 9876, test.Out().Read(), verbosity);
	
	sel.Write(4);
	test.Update();
	success &= TestState(i++, 3333333, test.Out().Read(), verbosity);
	
	sel.Write(8U);
	test.Update();
	success &= TestState(i++, 4444444, test.Out().Read(), verbosity);
	return success;
}

bool TestEncoder4(const Wire& a, const Wire& b, const Wire& c)
{
	Decoder<4> dec;
	dec.Connect({ &a, &b }, c);
	Encoder<4> test;
	test.Connect(dec.Out());
	dec.Update();
	test.Update();
	return c.On() ? (dec.Out().UnsignedRead() == 1 << test.Out().UnsignedRead()) : (test.Out().Read() == 0);
}


bool TestEncoder8(const Wire& a, const Wire& b, const Wire& c)
{
	Decoder<8> dec;
	dec.Connect({ &a, &b, &c }, Wire::ON);
	Encoder<8> test;
	test.Connect(dec.Out());
	dec.Update();
	test.Update();
	return dec.Out().UnsignedRead() == 1 << test.Out().UnsignedRead();
}

bool TestMultiplexer4(const Wire& a, const Wire& b)
{
	Multiplexer<4> test;
	test.Connect({ &Wire::OFF, &Wire::ON, &Wire::ON, &Wire::OFF }, { &a, &b });
	test.Update();
	return test.Out().On() == (a.On() ^ b.On());
}

bool TestMultiplexer8(const Wire& a, const Wire& b, const Wire& c)
{
	Multiplexer<8> test;
	test.Connect({ &Wire::ON, &Wire::OFF, &Wire::ON, &Wire::OFF, &Wire::ON, &Wire::OFF, &Wire::ON, &Wire::OFF }, { &a, &b, &c });
	test.Update();
	return test.Out().On() != a.On();
}

bool TestMatcher(const Bundle<8>& in)
{
	Matcher<4> test;
	Bundle<4> a = in.Range<4>(0);
	Bundle<4> b = in.Range<4>(4);
	test.Connect(a, b);
	test.Update();
	return test.Out().On() == (a.Read() == b.Read());
}

bool TestComparator(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Comparator<8> test;
	MagicBundle<8> num1, num2;
	test.Connect(num1, num2);
	for (const auto&[a, b] : std::map<int, int>({ { -12, -12 },{ -13, -12 },{ -11, -12 },{ 0, 0 },{ 62, 63 },{ 12, 0 } }))
	{
		num1.Write(a);
		num2.Write(b);
		test.Update();
		success &= TestState(i++, num1.Read() == num2.Read(), test.Equal().On(), verbosity);
		success &= TestState(i++, num1.Read() < num2.Read(), test.Less().On(), verbosity);
		success &= TestState(i++, num1.Read() <= num2.Read(), test.LessEqual().On(), verbosity);
		success &= TestState(i++, num1.Read() > num2.Read(), test.Greater().On(), verbosity);
		success &= TestState(i++, num1.Read() >= num2.Read(), test.GreaterEqual().On(), verbosity);
	}
	return success;
}

bool TestShifter(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Shifter<8> test;
	MagicBundle<8> num;
	MagicBundle<3> shift;
	Wire signext(false);
	Wire right(true);
	test.Connect(num, shift, right, signext);
	num.Write(224U);
	for (unsigned int s = 0; s < 8U; s++)
	{
		shift.Write(s);
		test.Update();
		success &= TestState(i++, (unsigned int)(224U / (1<<s)), test.Out().UnsignedRead(), verbosity);
	}
	signext.Set(true);
	num.Write(107);
	for (unsigned int s = 0; s < 8U; s++)
	{
		shift.Write(s);
		test.Update();
		success &= TestState(i++, (unsigned int)(107U / (1 << s)), test.Out().UnsignedRead(), verbosity);
	}

	num.Write(-113);
	for (unsigned int s = 0; s < 8U; s++)
	{
		shift.Write(s);
		test.Update();
		success &= TestState(i++, (int)floor(-113.0 / (1 << s)), test.Out().Read(), verbosity);
	}

	right.Set(false);
	num.Write(5);
	for (unsigned int s = 0; s < 8U; s++)
	{
		shift.Write(s);
		test.Update();
		success &= TestState(i++, (unsigned int)(5 << s) % 256, test.Out().UnsignedRead(), verbosity);
	}
	num.Write(-3);
	for (unsigned int s = 0; s < 6U; s++)
	{
		shift.Write(s);
		test.Update();
		success &= TestState(i++, (int)(-3 << s), test.Out().Read(), verbosity);
	}
	return success;
}

bool TestDecoder4(const Wire& a, const Wire& b, const Wire& c)
{
	Decoder<4> test;
	test.Connect({ &a, &b }, c);
	test.Update();
	return c.On() ? (test.Out().UnsignedRead() == pow2(Bundle<2>({ &a, &b }).UnsignedRead())) : test.Out().Read() == 0;
}

bool TestDecoder8(const Bundle<3>& in)
{
	Decoder<8> test;
	test.Connect(in, Wire::ON);
	test.Update();
	return test.Out().UnsignedRead() == pow2(in.UnsignedRead());
}

bool TestDecoder32(const Bundle<5>& in)
{
	Decoder<32> test;
	test.Connect(in, Wire::ON);
	test.Update();
	return test.Out().UnsignedRead() == pow2(in.UnsignedRead());
}

bool TestALU(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	ALU<8> alu;
	MagicBundle<8> a_reg;
	MagicBundle<8> b_reg;
	MagicBundle<4> sel;
	alu.Connect(a_reg, b_reg, sel);

	for (const auto&[a, b] : std::map<int, int>({ { -64, -64 },{ 0, 0 },{ 15, 9 },{ 11, 115 },{ 4, -121 } }))
	{
		a_reg.Write(a);
		b_reg.Write(b);

		if (verbosity == VERBOSE)
		{
			std::cout << i << ". Testing " << a << ", " << b << std::endl;
		}

		sel.Write(ALU_OPCODE::A);
		alu.Update();
		success &= TestState(i++, a, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_PLUS_ONE);
		alu.Update();
		success &= TestState(i++, a + 1, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_PLUS_B);
		alu.Update();
		success &= TestState(i++, a + b, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_PLUS_B_PLUS_ONE);
		alu.Update();
		success &= TestState(i++, a + b + 1, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_MINUS_B_MINUS_ONE);
		alu.Update();
		success &= TestState(i++, a - b - 1, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_MINUS_B);
		alu.Update();
		success &= TestState(i++, a - b, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_MINUS_ONE);
		alu.Update();
		success &= TestState(i++, a - 1, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_ALSO);
		alu.Update();
		success &= TestState(i++, a, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_AND_B);
		alu.Update();
		success &= TestState(i++, a & b, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_OR_B);
		alu.Update();
		success &= TestState(i++, a | b, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_XOR_B);
		alu.Update();
		success &= TestState(i++, a ^ b, alu.Out().Read(), verbosity);

		sel.Write(ALU_OPCODE::A_NOR_B);
		alu.Update();
		success &= TestState(i++, (~(a | b)), alu.Out().Read(), verbosity);
	}

	// Test shift
	a_reg.Write(224U);
	sel.Write(ALU_OPCODE::A_SHRL);
	for (unsigned int s = 0; s < 8U; s++)
	{
		b_reg.Write(s);
		alu.Update();
		success &= TestState(i++, (unsigned int)(224U / (1 << s)), alu.Out().UnsignedRead(), verbosity);
	}
	sel.Write(ALU_OPCODE::A_SHRL);
	a_reg.Write(107);
	for (unsigned int s = 0; s < 8U; s++)
	{
		b_reg.Write(s);
		alu.Update();
		success &= TestState(i++, (unsigned int)(107U / (1 << s)), alu.Out().UnsignedRead(), verbosity);
	}

	sel.Write(ALU_OPCODE::A_SHRA);
	a_reg.Write(-113);
	for (unsigned int s = 0; s < 8U; s++)
	{
		b_reg.Write(s);
		alu.Update();
		success &= TestState(i++, (int)floor(-113.0 / (1 << s)), alu.Out().Read(), verbosity);
	}

	sel.Write(ALU_OPCODE::A_SHLL);
	a_reg.Write(5);
	for (unsigned int s = 0; s < 8U; s++)
	{
		b_reg.Write(s);
		alu.Update();
		success &= TestState(i++, (unsigned int)(5 << s) % 256, alu.Out().UnsignedRead(), verbosity);
	}

	sel.Write(ALU_OPCODE::A_SHLL);
	a_reg.Write(-3);
	for (unsigned int s = 0; s < 6U; s++)
	{
		b_reg.Write(s);
		alu.Update();
		success &= TestState(i++, (int)(-3 << s), alu.Out().Read(), verbosity);
	}


	// Test flags
	a_reg.Write(125);
	b_reg.Write(2);

	sel.Write(ALU_OPCODE::A_PLUS_B);
	alu.Update();
	success &= TestState(i++, 127, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	sel.Write(ALU_OPCODE::A_PLUS_B_PLUS_ONE);
	alu.Update();
	success &= TestState(i++, -128, alu.Out().Read(), verbosity);
	success &= TestState(i++, true, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	a_reg.Write(-125);
	b_reg.Write(3);

	sel.Write(ALU_OPCODE::A_MINUS_B);
	alu.Update();
	success &= TestState(i++, -128, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	sel.Write(ALU_OPCODE::A_MINUS_B_MINUS_ONE);
	alu.Update();
	success &= TestState(i++, 127, alu.Out().Read(), verbosity);
	success &= TestState(i++, true, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	a_reg.Write(3);
	b_reg.Write(3);

	sel.Write(ALU_OPCODE::A_MINUS_B);
	alu.Update();
	success &= TestState(i++, 0, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Zero().On(), verbosity);

	sel.Write(ALU_OPCODE::A_MINUS_B_MINUS_ONE);
	alu.Update();
	success &= TestState(i++, -1, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	a_reg.Write(11);
	b_reg.Write(10);

	sel.Write(ALU_OPCODE::A_MINUS_B);
	alu.Update();
	success &= TestState(i++, 1, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	a_reg.Write(10);
	b_reg.Write(10);

	sel.Write(ALU_OPCODE::A_MINUS_B);
	alu.Update();
	success &= TestState(i++, 0, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Zero().On(), verbosity);

	a_reg.Write(9);
	b_reg.Write(10);

	sel.Write(ALU_OPCODE::A_MINUS_B);
	alu.Update();
	success &= TestState(i++, -1, alu.Out().Read(), verbosity);
	success &= TestState(i++, false, alu.Flags().Overflow().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Carry().On(), verbosity);
	success &= TestState(i++, true, alu.Flags().Negative().On(), verbosity);
	success &= TestState(i++, false, alu.Flags().Zero().On(), verbosity);

	return success;
}

bool TestRegisterFile(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	RegisterFile<32, 8> test;
	MagicBundle<3> addr1, addr2, addrw;
	MagicBundle<32> data;
	Wire write;
	test.Connect(addr1, addr2, addrw, data, write);

	addr1.Write(4U);
	addr2.Write(2U);
	addrw.Write(4U);
	data.Write(123456U);
	test.Update();
	success &= TestState(i++, 0, test.Out1().Read(), verbosity);
	success &= TestState(i++, 0, test.Out2().Read(), verbosity);

	write.Set(true);
	test.Update();
	success &= TestState(i++, 123456, test.Out1().Read(), verbosity);
	success &= TestState(i++, 0, test.Out2().Read(), verbosity);

	data.Write(987654321U);
	write.Set(false);
	test.Update();
	success &= TestState(i++, 123456, test.Out1().Read(), verbosity);
	success &= TestState(i++, 0, test.Out2().Read(), verbosity);

	addrw.Write(2U);
	write.Set(true);
	test.Update();
	success &= TestState(i++, 123456, test.Out1().Read(), verbosity);
	success &= TestState(i++, 987654321, test.Out2().Read(), verbosity);

	data.Write(-7281);
	write.Set(false);
	test.Update();
	success &= TestState(i++, 123456, test.Out1().Read(), verbosity);
	success &= TestState(i++, 987654321, test.Out2().Read(), verbosity);

	test.Update();
	test.Update();
	addrw.Write(7U);
	write.Set(true);
	test.Update();
	write.Set(false);
	addr2.Write(7U);
	addr1.Write(4U);
	test.Update();
	success &= TestState(i++, 123456, test.Out1().Read(), verbosity);
	success &= TestState(i++, -7281, test.Out2().Read(), verbosity);
	
	return success;
}

bool TestMasker(Verbosity verbosity)
{
	int i = 0;
	bool success = true;

	Masker<32> test;
	MagicBundle<32> maskee, base, mask;
	test.Connect(maskee, base, mask);

	maskee.Write(0x12345678);
	base.Write(0xaaaaaaaaU);
	mask.Write(0);
	test.Update();
	success &= TestState(i++, 0xaaaaaaaaU, test.Out().UnsignedRead(), verbosity);

	mask.Write(0x00ffff00);
	test.Update();
	success &= TestState(i++, 0xaa3456aaU, test.Out().UnsignedRead(), verbosity);

	mask.Write(0xff0000f0U);
	test.Update();
	success &= TestState(i++, 0x12aaaa7aU, test.Out().UnsignedRead(), verbosity);

	mask.Write(0xffffffffU);
	test.Update();
	success &= TestState(i++, 0x12345678U, test.Out().UnsignedRead(), verbosity);

	return success;
}

bool TestMultiplier(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Multiplier<8> test;
	MagicBundle<8> a_reg, b_reg;
	test.Connect(a_reg, b_reg, Wire::ON);

	for (const auto&[a, b] : std::map<int, int>({ { -64, -64 },{ 0, 0 },{ 15, 9 },{ 11, 115 },{ 4, -121 }, {47, 63}, {127, 127 }, {-128, -128}, {-127, 127} }))
	{
		a_reg.Write(a);
		b_reg.Write(b);

		if (verbosity == VERBOSE)
		{
			std::cout << i << ". Testing " << a << ", " << b << std::endl;
		}
		test.Update();
		success &= TestState(i++, (unsigned int)(a*b) & 0xFF, test.OutLo().UnsignedRead(), verbosity);
		success &= TestState(i++, (a*b) >> 8, test.OutHi().Read(), verbosity);
	} 
	return success;
}

bool TestCacheLine(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	CacheLine<16, 4, 20> test;
	MagicBundle<2> offset;
	MagicBundle<16> dataword;
	MagicBundle<64> dataline;
	MagicBundle<20> tag;
	MagicBundle<16> writemask;
	Wire writeline(false);

	test.Connect(tag, offset, writemask, dataword, writeline, dataline, Wire::ON);

	dataline.Write(0x1122334455667788);
	tag.Write(987);
	writemask.Write(0);
	writeline.Set(true);
	test.Update();
	success &= TestState(i++, 0x1122334455667788, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	dataline.Write(100000);
	tag.Write(123);
	writeline.Set(false);
	test.Update();
	success &= TestState(i++, false, test.CacheHit().On(), verbosity);

	tag.Write(987);
	test.Update();
	success &= TestState(i++, 0x1122334455667788, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	dataword.Write(0xbabeU);
	writemask.Write(0xffffU);
	writeline.Set(false);
	offset.Write(1U);
	test.Update();
	success &= TestState(i++, 0x11223344babe7788, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	dataword.Write(0xcafeU);
	offset.Write(2U);
	writemask.Write(0xff00U);
	test.Update();
	success &= TestState(i++, 0x1122ca44babe7788, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	dataword.Write(0xbeefU);
	offset.Write(3U);
	writemask.Write(0x00ffU);
	test.Update();
	success &= TestState(i++, 0x11efca44babe7788, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	dataword.Write(0xaaaaU);
	offset.Write(0U);
	writemask.Write(0x0fffU);
	test.Update();
	success &= TestState(i++, 0x11efca44babe7aaa, test.OutLine().ReadLong(), verbosity);
	success &= TestState(i++, true, test.CacheHit().On(), verbosity);

	return success;
}

bool TestCircularBuffer(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	CircularBuffer<32, 4> test;

	MagicBundle<32> reg;
	Wire read(false);
	Wire write(false);

	test.Connect(reg, read, write);

	reg.Write(123456);
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);

	write.Set(true);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);

	reg.Write(11);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);

	reg.Write(22222);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	
	read.Set(true);
	write.Set(false);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 123456, test.Out().Read(), verbosity);

	read.Set(false);
	write.Set(true);
	reg.Write(3333333);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);

	reg.Write(4444444);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, true, test.Full().On(), verbosity);

	reg.Write(5555555);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, true, test.Full().On(), verbosity);

	read.Set(true);
	write.Set(false);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 11, test.Out().Read(), verbosity);

	read.Set(false);
	write.Set(true);
	reg.Write(9876);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, true, test.Full().On(), verbosity);

	read.Set(true);
	write.Set(false);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 22222, test.Out().Read(), verbosity);

	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 3333333, test.Out().Read(), verbosity);

	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 4444444, test.Out().Read(), verbosity);
	
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 9876, test.Out().Read(), verbosity);

	read.Set(false);
	write.Set(true);
	reg.Write(11111);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 9876, test.Out().Read(), verbosity);

	read.Set(true);
	reg.Write(2222);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 11111, test.Out().Read(), verbosity);

	reg.Write(333333);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 2222, test.Out().Read(), verbosity);

	write.Set(false);
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 333333, test.Out().Read(), verbosity);
	return success;
}

bool TestCircularBuffer1(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	CircularBuffer<32, 1> test;

	MagicBundle<32> reg;
	Wire read(false);
	Wire write(false);

	test.Connect(reg, read, write);

	reg.Write(123456);
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);

	write.Set(true);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, true, test.Full().On(), verbosity);

	reg.Write(11);
	test.Update();
	success &= TestState(i++, true, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, true, test.Full().On(), verbosity);

	read.Set(true);
	write.Set(false);
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 123456, test.Out().Read(), verbosity);
		
	test.Update();
	success &= TestState(i++, false, test.NonEmpty().On(), verbosity);
	success &= TestState(i++, false, test.Full().On(), verbosity);
	success &= TestState(i++, 123456, test.Out().Read(), verbosity);
		
	return success;
}

bool TestRequestBuffer(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	RequestBuffer<32, 10, 2, 4> test;

	MagicBundle<32> data;
	MagicBundle<10> addr;
	Wire write(false);	
	Wire read(false);	

	test.Connect(addr, data, write, read);

	data.Write(11);
	addr.Write(10);
	write.Set(false);

	test.Update();
	success &= TestState(i++, false, test.WriteFull().On(), verbosity);
	success &= TestState(i++, false, test.ReadPending().On(), verbosity);
	success &= TestState(i++, false, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, false, test.PoppedRead().On(), verbosity);

	write.Set(true);
	test.Update();
	success &= TestState(i++, false, test.WriteFull().On(), verbosity);
	success &= TestState(i++, false, test.ReadPending().On(), verbosity);
	success &= TestState(i++, false, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, false, test.PoppedRead().On(), verbosity);

	data.Write(2222);
	addr.Write(31);
	test.Update();
	success &= TestState(i++, true, test.WriteFull().On(), verbosity);
	success &= TestState(i++, false, test.ReadPending().On(), verbosity);
	success &= TestState(i++, false, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, false, test.PoppedRead().On(), verbosity);

	write.Set(false);
	read.Set(true);
	addr.Write(128);
	data.Write(3333333);
	test.Update(); // 4th one, will pop as well as add this read
	success &= TestState(i++, false, test.WriteFull().On(), verbosity);
	success &= TestState(i++, true, test.ReadPending().On(), verbosity);
	success &= TestState(i++, true, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, false, test.PoppedRead().On(), verbosity);
	success &= TestState(i++, 11, test.OutWrite().Data().Read(), verbosity);
	success &= TestState(i++, 10, test.OutWrite().Addr().Read(), verbosity);
	success &= TestState(i++, 0, test.OutRead().Read(), verbosity);
	
	addr.Write(3);
	for (int wait = 0; wait < 3; wait++)
	{
		test.Update(); // Attempt another read, does nothing
		success &= TestState(i++, false, test.WriteFull().On(), verbosity);
		success &= TestState(i++, true, test.ReadPending().On(), verbosity);
		success &= TestState(i++, true, test.PoppedWrite().On(), verbosity);
		success &= TestState(i++, false, test.PoppedRead().On(), verbosity);
		success &= TestState(i++, 11, test.OutWrite().Data().Read(), verbosity);
		success &= TestState(i++, 10, test.OutWrite().Addr().Read(), verbosity);
		success &= TestState(i++, 0, test.OutRead().Read(), verbosity);
	}
	test.Update();
	success &= TestState(i++, false, test.WriteFull().On(), verbosity);
	success &= TestState(i++, true, test.ReadPending().On(), verbosity);
	success &= TestState(i++, true, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, false, test.PoppedRead().On(), verbosity);
	success &= TestState(i++, 2222, test.OutWrite().Data().Read(), verbosity);
	success &= TestState(i++, 31, test.OutWrite().Addr().Read(), verbosity);
	success &= TestState(i++, 0, test.OutRead().Read(), verbosity);
	for (int wait = 0; wait < 3; wait++)
	{
		test.Update();
		success &= TestState(i++, false, test.WriteFull().On(), verbosity);
		success &= TestState(i++, true, test.ReadPending().On(), verbosity);
		success &= TestState(i++, true, test.PoppedWrite().On(), verbosity);
		success &= TestState(i++, false, test.PoppedRead().On(), verbosity);
		success &= TestState(i++, 2222, test.OutWrite().Data().Read(), verbosity);
		success &= TestState(i++, 31, test.OutWrite().Addr().Read(), verbosity);
		success &= TestState(i++, 0, test.OutRead().Read(), verbosity);
	}
	test.Update();
	success &= TestState(i++, false, test.WriteFull().On(), verbosity);
	success &= TestState(i++, false, test.ReadPending().On(), verbosity);
	success &= TestState(i++, false, test.PoppedWrite().On(), verbosity);
	success &= TestState(i++, true, test.PoppedRead().On(), verbosity);
	success &= TestState(i++, 0, test.OutWrite().Data().Read(), verbosity);
	success &= TestState(i++, 0, test.OutWrite().Addr().Read(), verbosity);
	success &= TestState(i++, 128, test.OutRead().Read(), verbosity);
				
	return success;
}

bool RunAllTests()
{
	bool success = true;

	RUN_AUTO_TEST(TestOneWireComponent, TestInverter, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestInverter3, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestAndGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestAndGate4, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestOrGate4, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNandGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestOrGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNorGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestXorGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestXNorGate, FAIL_ONLY);
	RUN_TEST(TestMultiGate, FAIL_ONLY);
	RUN_TEST(TestMultiGateN, FAIL_ONLY);
	RUN_TEST(TestMultiGateNBitwise, FAIL_ONLY);
	RUN_TEST(TestSRLatch, FAIL_ONLY);
	RUN_TEST(TestJKFlipFlop, FAIL_ONLY);
	RUN_TEST(TestDFlipFlop, FAIL_ONLY);
	RUN_TEST(TestDFlipFlopReset, FAIL_ONLY);
	RUN_TEST(TestBundle, FAIL_ONLY);
	RUN_TEST(TestRegister, FAIL_ONLY);
	RUN_TEST(TestCounter, FAIL_ONLY);
	RUN_TEST(TestFreqSwitcher, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestFullAdder, FAIL_ONLY);
	RUN_AUTO_TEST(TestOneWireComponent, TestMultiplexer2, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestMultiplexer4, FAIL_ONLY);
	RUN_TEST(TestMuxBundle, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestEncoder4, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestEncoder8, FAIL_ONLY);
	RUN_TEST(TestSelectBundle, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestMultiplexer8, FAIL_ONLY);
	RUN_TEST(TestComparator, FAIL_ONLY);
	RUN_TEST(TestShifter, FAIL_ONLY);
	RUN_TEST(TestMasker, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestMatcher, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestDecoder4, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestDecoder8, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestDecoder32, FAIL_ONLY);
	RUN_TEST(TestALU, FAIL_ONLY);
	RUN_TEST(TestRegisterFile, FAIL_ONLY);
	RUN_TEST(TestCacheLine, FAIL_ONLY);
	RUN_TEST(TestMultiplier, FAIL_ONLY);
	RUN_TEST(TestCircularBuffer, FAIL_ONLY);
	RUN_TEST(TestCircularBuffer1, FAIL_ONLY);
	RUN_TEST(TestRequestBuffer, FAIL_ONLY);
	return success;
}
#endif
