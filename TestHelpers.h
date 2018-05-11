#pragma once

#include <iostream>
#include <cmath>
#include "Wire.h"

enum Verbosity
{
	SILENT,
	FAIL_ONLY,
	VERBOSE
};

#define RUN_AUTO_TEST(runner, unit_test, v) std::cout << "Testing " << #unit_test << std::endl; success &= runner(unit_test, v);
#define RUN_TEST(unit_test, v) std::cout << "Testing " << #unit_test << std::endl; success &= unit_test(v);

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

template <typename T>
bool TestState(int i, T val1, T val2, Verbosity verbosity)
{
	bool pass = val1 == val2;
	if (verbosity == VERBOSE || (verbosity == FAIL_ONLY && !pass))
	{
		std::cout << i << ".\t" << (pass ? "PASS: " : "FAIL: ") << std::endl;
	}
	return pass;
}