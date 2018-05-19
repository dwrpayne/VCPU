#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"
#include "Tests/ComponentCost.h"

void RunCPU(std::string filename)
{
	Assembler assembler(filename);
	Debugger debugger(assembler.GetProgram());
	debugger.Start();
}


int main(int argc, char** argv)
{
	bool success = true;
	PrintComponentCosts();
#ifdef DEBUG
	success &= RunAllTests();
#endif
	success &= RunCPUTests();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;

	//RunCPU("testops.vasm");
}