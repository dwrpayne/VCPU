#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"
#include "Tests/ComponentCost.h"


void Test()
{
	bool success = true;
	PrintComponentCosts();
#ifdef DEBUG
	success &= RunAllTests();
#endif
	success &= RunCPUTests();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;
}

void RunCPU(std::string filename)
{
	Debugger debugger(filename);
	debugger.SetInstructionPrint(true);
	debugger.SetRegisterPrint(true);
	debugger.Start();
}

int main(int argc, char** argv)
{
	//Test();
	RunCPU("primes2.vasm");
}