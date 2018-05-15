#include "Tests/Tests.h"
#include "Tests/TestCPU.h"
#include "CPU/CPU.h"
#include "Tools/Debugger.h"

void RunCPU()
{
	CPU* pcpu = new CPU();
	CPU& cpu = *pcpu;
	Debugger debugger(cpu);
	debugger.LoadProgram();
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