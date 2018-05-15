#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"
#include "Tools/Assembler.h"

void RunCPU()
{
	CPU* pcpu = new CPU();
	CPU& cpu = *pcpu;

	Assembler assembler("fib.vasm");
	
	Debugger debugger(cpu, assembler.GetProgram());
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

	RunCPU();
}