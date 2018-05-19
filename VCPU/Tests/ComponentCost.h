#include "CPU/CPU.h"

#define _PRINT_COST(obj) std::cout << obj.Cost() << "\t" << typeid(obj).name() << std::endl;
#define PRINT_COST(type) {type t; _PRINT_COST(t);}
#define PRINT_COST_NEW(type) {type* t = new type(); _PRINT_COST((*t)); delete t;}

void PrintComponentCosts()
{
	typedef MultiGate<XorGate, 32> MultiXor32;
	typedef MuxBundle<32, 2> MuxBundle322;
	typedef MuxBundle<32, 4> MuxBundle324;
	typedef MuxBundle<6, 4> MuxBundle64;
	typedef RegisterFile<32, 32> RegisterFile3232;

	PRINT_COST(AndGate);
	PRINT_COST(Inverter);
	PRINT_COST(OrGate);
	PRINT_COST(NandGate);
	PRINT_COST(NorGate);
	PRINT_COST(XorGate);
	PRINT_COST(AndGateN<4>);
	PRINT_COST(AndGateN<32>);
	PRINT_COST(InverterN<32>);
	PRINT_COST(NandGateN<32>);
	PRINT_COST(MultiXor32);
	PRINT_COST(SRLatch);
	PRINT_COST(JKFlipFlop);
	PRINT_COST(DFlipFlop);
	PRINT_COST(Register<4>);
	PRINT_COST(Register<32>);
	PRINT_COST(Counter<4>);
	PRINT_COST(FullAdder);
	PRINT_COST(FullAdderN<32>);
	PRINT_COST(OverflowDetector);
	PRINT_COST(Adder<32>);
	PRINT_COST(Multiplexer<2>);
	PRINT_COST(Multiplexer<8>);
	PRINT_COST(Decoder<32>);
	PRINT_COST(Decoder<1024>);
	PRINT_COST(MuxBundle322);
	PRINT_COST(MuxBundle324);
	PRINT_COST(MuxBundle64);
	PRINT_COST(ALU<32>);
	PRINT_COST(RegisterFile3232);

	PRINT_COST_NEW(CPU::InsMemory);
	PRINT_COST_NEW(CPU::MainMemory);
	PRINT_COST_NEW(CPU);
}