#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"

void RunCPU(std::string filename)
{
	CPU* pcpu = new CPU();
	Assembler assembler(filename);
	Debugger debugger(*pcpu, assembler.GetProgram());
	debugger.Start();

}


int main(int argc, char** argv)
{
	bool success = true;
#ifdef DEBUG
	success &= RunAllTests();
#endif
	success &= RunCPUTests();
	std::cout << (success ? "All tests passed!" : "Some tests failed!") << std::endl;

	RunCPU("testops.vasm");
}