#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"
#include "Tests/ComponentCost.h"

void Test()
{
	bool success = true;
#ifdef DEBUG
	PrintComponentCosts();
	success &= RunAllTests();
#endif
	success &= RunCPUTests();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;
}

void RunCPU(std::string filename)
{
	Debugger debugger(filename, Debugger::MINIMAL);
	debugger.Start();
}

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		Test();
		RunCPU("testecho.vasm");
	}
	else
	{
		RunCPU(argv[1]);
	}
}
