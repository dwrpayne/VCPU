#include "TestHelpers.h"

void print() {
	std::cout << std::endl;
}

bool TestOneWireComponent(bool(*test_func)(const Wire &), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::ON);
	return success;
}

bool TestTwoWireComponent(bool(*test_func)(const Wire &, const Wire &), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, Wire::OFF, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::ON, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::OFF, Wire::ON);
	success &= Test(verbosity, test_func, Wire::ON, Wire::ON);
	return success;
}

bool TestThreeWireComponent(bool(*test_func)(const Wire &, const Wire &, const Wire &), Verbosity verbosity)
{
	bool success = true;
	success &= Test(verbosity, test_func, Wire::OFF, Wire::OFF, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::ON, Wire::OFF, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::OFF, Wire::ON, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::ON, Wire::ON, Wire::OFF);
	success &= Test(verbosity, test_func, Wire::OFF, Wire::OFF, Wire::ON);
	success &= Test(verbosity, test_func, Wire::ON, Wire::OFF, Wire::ON);
	success &= Test(verbosity, test_func, Wire::OFF, Wire::ON, Wire::ON);
	success &= Test(verbosity, test_func, Wire::ON, Wire::ON, Wire::ON);
	return success;
}
