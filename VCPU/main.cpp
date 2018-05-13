#include "Tests/Tests.h"
#include "Tests/TestCPU.h"

int main(int argc, char** argv)
{
	bool success = true;
#ifdef DEBUG
	success &= RunAllTests();
#endif
	TestCPU();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;
}