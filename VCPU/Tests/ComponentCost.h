#include "CPU/CPU.h"
#include "CPU/Cache.h"

#define _PRINT_COST(obj) std::cout << Wire::WireCount() - cnt << "\t" << typeid(obj).name() << std::endl;
#define PRINT_COST(type) {int cnt = Wire::WireCount(); type t; _PRINT_COST(t);}
#define PRINT_COST_NEW(type) {int cnt = Wire::WireCount(); type* t = new type(); _PRINT_COST((*t)); delete t;}
#define PRINT_COST_NEW_MEM(type) {int cnt = Wire::WireCount(); type* t = new type(true); _PRINT_COST((*t)); delete t;}

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
	typedef CacheLine<256, 8> CacheLine2568;

	PRINT_COST_NEW(AndGate);
	PRINT_COST_NEW(Inverter);
	PRINT_COST_NEW(OrGate);
	PRINT_COST_NEW(NandGate);
	PRINT_COST_NEW(NorGate);
	PRINT_COST_NEW(XorGate);
	PRINT_COST_NEW(AndGateN<4>);
	PRINT_COST_NEW(AndGateN<32>);
	PRINT_COST_NEW(InverterN<32>);
	PRINT_COST_NEW(NandGateN<32>);
	PRINT_COST_NEW(MultiXor32);
	PRINT_COST_NEW(SRLatch);
	PRINT_COST_NEW(JKFlipFlop);
	PRINT_COST_NEW(DFlipFlop);
	PRINT_COST_NEW(Register<4>);
	PRINT_COST_NEW(Register<32>);
	PRINT_COST_NEW(Counter<4>);
	PRINT_COST_NEW(FullAdder);
	PRINT_COST_NEW(FullAdderN<32>);
	PRINT_COST_NEW(Multiplier<32>);
	PRINT_COST_NEW(Comparator<32>);
	PRINT_COST_NEW(Matcher<32>);
	PRINT_COST_NEW(LeftShifter<32>);
	PRINT_COST_NEW(Masker<32>);
	PRINT_COST_NEW(OverflowDetector);
	PRINT_COST_NEW(Adder<32>);
	PRINT_COST_NEW(Shifter<32>);
	PRINT_COST_NEW(Multiplexer<2>);
	PRINT_COST_NEW(Multiplexer<8>);
	PRINT_COST_NEW(Decoder<32>);
	PRINT_COST_NEW(Decoder<1024>);
	PRINT_COST_NEW(MuxBundle322);
	PRINT_COST_NEW(MuxBundle324);
	PRINT_COST_NEW(MuxBundle64);
	PRINT_COST_NEW(MuxBundle32128);
	PRINT_COST_NEW(RegArray32128);
	PRINT_COST_NEW(ALU<32>);
	PRINT_COST_NEW(RegisterFile3232);
	PRINT_COST_NEW(CacheLine2568);
	PRINT_COST_NEW(OpcodeDecoder);
	PRINT_COST_NEW(CPU::InsCache);
	PRINT_COST_NEW(CPU::MainCache);
	PRINT_COST_NEW_MEM(CPU::InsMemory);
	PRINT_COST_NEW_MEM(CPU::MainMemory);
	PRINT_COST_NEW(CPU::RegFile);
	PRINT_COST_NEW(CPU);	
}