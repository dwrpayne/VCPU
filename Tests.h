#pragma once
#include <iostream>
#include <cmath>
#include "AndGate.h"
#include "OrGate.h"
#include "NandGate.h"
#include "NorGate.h"
#include "Inverter.h"
#include "SRLatch.h"
#include "JKFlipFlop.h"
#include "DFlipFlop.h"

enum Verbosity
{
	SILENT,
	FAIL_ONLY,
	VERBOSE
};

#define WIRE(b) (b ? WIRE_ON : WIRE_OFF)


void print() {
	std::cout << std::endl;
}
template<class T, class... Args>
void print(T t1, Args... args) {
	std::cout << t1 << " ";
	print(args...);
}

template<typename FuncType, typename ...Args>
bool Test(Verbosity verbosity, FuncType && func, Args && ...args)
{
	bool pass = func(args...);
	if (verbosity == VERBOSE || (verbosity == FAIL_ONLY && !pass))
	{
		std::cout << "\t" << (pass ? "PASS: " : "FAIL: ") << " for input ";
		print(args...);
	}
	return pass;
}

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

bool TestInverter(const Wire& a)
{
	Inverter test;
	test.Connect(a);
	test.Update();
	bool out = test.Out().On();
	return out != a.On();
}

bool TestState(int i, const Wire& wire, bool state, Verbosity verbosity)
{
	bool pass = wire.On() == state;
	if (verbosity == VERBOSE || (verbosity == FAIL_ONLY && !pass))
	{
		std::cout << i << ".\t" << (pass ? "PASS: " : "FAIL: ") << std::endl;
	}
	return pass;
}

bool TestSRLatch(Verbosity verbosity)
{
	SRLatch test;
	test.Connect(WIRE_OFF, WIRE_ON); 
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_OFF); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_OFF); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);
	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_OFF, WIRE_OFF); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	return success;
}

bool TestJKFlipFlop(Verbosity verbosity)
{
	JKFlipFlop test;
	test.Connect(WIRE_OFF, WIRE_ON);
	test.Update();
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_OFF); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_OFF); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);
	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_OFF, WIRE_OFF); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_OFF); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_OFF, WIRE_OFF); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_OFF, WIRE_OFF); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	return success;
}

bool TestDFlipFlop(Verbosity verbosity)
{
	DFlipFlop test;
	test.Connect(WIRE_OFF, WIRE_ON);
	test.Update();
	const Wire& q = test.Q();
	const Wire& notq = test.NotQ();

	bool success = true;

	int i = 0;
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_ON, WIRE_ON); test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Update();
	success &= TestState(i++, q, true, verbosity);
	success &= TestState(i++, notq, false, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Connect(WIRE_OFF, WIRE_ON); test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);

	test.Update();
	success &= TestState(i++, q, false, verbosity);
	success &= TestState(i++, notq, true, verbosity);
	
	return success;
}

bool TestOneWireComponent(bool(*test_func)(const Wire&), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_ON);
	return success;
}

bool TestTwoWireComponent(bool(*test_func)(const Wire&, const Wire&), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_ON);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_ON);
	return success;
}

bool TestThreeWireComponent(bool(*test_func)(const Wire&, const Wire&, const Wire&), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_OFF, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_OFF, WIRE_ON);
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_ON, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_OFF, WIRE_ON, WIRE_ON);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_OFF, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_OFF, WIRE_ON);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_ON, WIRE_OFF);
	success &= Test(verbosity, test_func, WIRE_ON, WIRE_ON, WIRE_ON);
	return success;
}

#define RUN_AUTO_TEST(runner, unit_test, v) std::cout << "Testing " << #unit_test << std::endl; success &= runner(unit_test, v);
#define RUN_TEST(unit_test, v) std::cout << "Testing " << #unit_test << std::endl; success &= unit_test(v);

bool RunAllTests()
{
	bool success = true;
	RUN_AUTO_TEST(TestOneWireComponent, TestInverter, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestAndGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNandGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestOrGate, FAIL_ONLY);
	RUN_AUTO_TEST(TestTwoWireComponent, TestNorGate, FAIL_ONLY);
	RUN_TEST(TestSRLatch, FAIL_ONLY);
	RUN_TEST(TestJKFlipFlop, FAIL_ONLY);
	RUN_TEST(TestDFlipFlop, FAIL_ONLY);
	return success;
}
