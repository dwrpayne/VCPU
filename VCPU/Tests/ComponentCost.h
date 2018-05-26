#include "CPU/CPU.h"
#include "CPU/Cache.h"

#define _PRINT_COST(obj) std::cout << Wire::WireCount() - cnt << "\t" << typeid(obj).name() << std::endl;
#define PRINT_COST(type) {int cnt = Wire::WireCount(); type t; _PRINT_COST(t);}
#define PRINT_COST_NEW(type) {int cnt = Wire::WireCount(); type* t = new type(); _PRINT_COST((*t)); delete t;}

void PrintComponentCosts()
{
	typedef MultiGate<XorGate, 32> MultiXor32;
	typedef MuxBundle<32, 2> MuxBundle322;
	typedef MuxBundle<32, 4> MuxBundle324;
	typedef MuxBundle<6, 4> MuxBundle64;
	typedef MuxBundle<256, 16> MuxBundle328;
	typedef MuxBundle<32, 128> MuxBundle32128;
	typedef MuxBundle<32, 2048> MuxBundle322048;
	typedef MuxBundle<256, 256> MuxBundle256256;
	typedef std::array<Register<32>,128> RegArray32128;
	typedef std::array<Register<32>, 2048> RegArray322048;
	typedef std::array<Register<256>, 256> RegArray256256;
	typedef RegisterFile<32, 32> RegisterFile3232;
	typedef CacheLine<32, 8, 13> CacheLine32813;
	typedef Cache<32, 512, 256> Cache32512256;

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
	PRINT_COST(Comparator<32>);
	PRINT_COST(Matcher<32>);
	PRINT_COST(OverflowDetector);
	PRINT_COST(Adder<32>);
	PRINT_COST(Shifter<32>);
	PRINT_COST(Multiplexer<2>);
	PRINT_COST(Multiplexer<8>);
	PRINT_COST(Decoder<32>);
	PRINT_COST(Decoder<1024>);
	PRINT_COST(MuxBundle322);
	PRINT_COST(MuxBundle324);
	PRINT_COST(MuxBundle64);
	PRINT_COST_NEW(MuxBundle32128);
	PRINT_COST_NEW(MuxBundle322048);
	PRINT_COST_NEW(MuxBundle256256);
	PRINT_COST_NEW(RegArray32128);
	PRINT_COST_NEW(RegArray322048);
	PRINT_COST_NEW(RegArray256256);
	PRINT_COST(ALU<32>);
	PRINT_COST(RegisterFile3232);
	PRINT_COST(CacheLine32813);

	PRINT_COST_NEW(CPU::InsCache);
	PRINT_COST_NEW(CPU::MainCache);
	PRINT_COST_NEW(CPU::RegFile);
	PRINT_COST_NEW(CPU);
}