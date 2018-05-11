#include "Tests.h"

int main(int argc, char** argv)
{
	bool success = RunAllTests();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;
}