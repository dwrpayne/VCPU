#pragma once
#include <iostream>
#include <cmath>
#include <map>
#include "TestHelpers.h"
#include "MagicBundle.h"
#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "MultiGate.h"
#include "XorGate.h"
#include "Inverter.h"
#include "SRLatch.h"
#include "JKFlipFlop.h"
#include "DFlipFlop.h"
#include "Bundle.h"
#include "Register.h"
#include "FullAdder.h"
#include "Adder.h"
#include "Multiplexer.h"
#include "MuxBundle.h"
#include "Decoder.h"


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

bool TestAndGate3(const Wire& a, const Wire& b, const Wire& c)
{
	AndGateN<3> test;
	test.Connect({ &a, &b, &c });
	test.Update();
	return test.Out().On() == (a.On() && b.On() && c.On());
}

bool TestXorGateN(Verbosity verbosity)
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
	JKFlipFlop test;
	test.Connect(Wire::OFF, Wire::ON);
	test.Update();
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

bool TestAdder(Verbosity verbosity)
{
	bool success = true;
	int i = 0;

	Adder<8> adder;
	MagicBundle<8> a_reg;
	MagicBundle<8> b_reg;
	MagicBundle<3> sel;
	adder.Connect(a_reg, b_reg, sel);

	for (const auto&[a, b] : std::map<int, int>({ { -64, -64 },{ 0, 0 }, {15, 9}, { 11, 115 },{ 4, -121 } }))
	{
		a_reg.Write(a);
		b_reg.Write(b);

		if (verbosity == VERBOSE)
		{
			std::cout << i << ". Testing " << a << ", " << b << std::endl;
		}

		sel.Write(0U);
		adder.Update();
		success &= TestState(i++, a, adder.S().Read(), verbosity);

		sel.Write(1U);
		adder.Update();
		success &= TestState(i++, a + 1, adder.S().Read(), verbosity);

		sel.Write(2U);
		adder.Update();
		success &= TestState(i++, a + b, adder.S().Read(), verbosity);

		sel.Write(3U);
		adder.Update();
		success &= TestState(i++, a + b + 1, adder.S().Read(), verbosity);

		sel.Write(4U);
		adder.Update();
		success &= TestState(i++, a - b - 1, adder.S().Read(), verbosity);

		sel.Write(5U);
		adder.Update();
		success &= TestState(i++, a - b, adder.S().Read(), verbosity);

		sel.Write(6U);
		adder.Update();
		success &= TestState(i++, a - 1, adder.S().Read(), verbosity);

		sel.Write(7U);
		adder.Update();
		success &= TestState(i++, a, adder.S().Read(), verbosity);
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
	test.Connect({ MagicBundle<32>(12345), MagicBundle<32>(9876) }, { &sel });

	test.Update();
	success &= TestState(i++, 12345, test.Out().Read(), verbosity);

	sel.Set(true);
	test.Update();
	success &= TestState(i++, 9876, test.Out().Read(), verbosity);
	return success;
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

bool TestDecoder4(const Wire& a, const Wire& b)
{
	Decoder<4> test;
	test.Connect({ &a, &b });
	test.Update();
	return test.Out().UnsignedRead() == pow2(Bundle<2>({ &a, &b }).UnsignedRead());
}

bool TestDecoder8(const Bundle<3>& in)
{
	Decoder<8> test;
	test.Connect(in);
	test.Update();
	return test.Out().UnsignedRead() == pow2(in.UnsignedRead());
}

bool TestDecoder32(const Bundle<5>& in)
{
	Decoder<32> test;
	test.Connect(in);
	test.Update();
	return test.Out().UnsignedRead() == pow2(in.UnsignedRead());
}

bool RunAllTests()
{
	bool success = true;
	RUN_AUTO_TEST(TestOneWireComponent, TestInverter, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestInverter3, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestAndGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestAndGate3, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNandGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestOrGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNorGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestXorGate, FAIL_ONLY);
	RUN_TEST(TestXorGateN, FAIL_ONLY);
	RUN_TEST(TestSRLatch, FAIL_ONLY);
	RUN_TEST(TestJKFlipFlop, FAIL_ONLY);
	RUN_TEST(TestDFlipFlop, FAIL_ONLY);
	RUN_TEST(TestBundle, FAIL_ONLY);
	RUN_TEST(TestRegister, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestFullAdder, FAIL_ONLY);
	RUN_TEST(TestAdder, FAIL_ONLY);
	RUN_AUTO_TEST(TestOneWireComponent, TestMultiplexer2, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestMultiplexer4, FAIL_ONLY);
	RUN_TEST(TestMuxBundle, FAIL_ONLY);
	RUN_AUTO_TEST(TestThreeWireComponent, TestMultiplexer8, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestDecoder4, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestDecoder8, FAIL_ONLY);
	RUN_AUTO_TEST(TestBundleComponent, TestDecoder32, FAIL_ONLY);
	return success;
}
