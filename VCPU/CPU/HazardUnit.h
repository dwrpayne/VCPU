#pragma once

#include "Component.h"
#include "Bundle.h"
#include "MuxBundle.h"
#include "Matcher.h"
#include "OrGate.h"

class HazardUnit : public Component
{
public:
	static const int ADDR = 5;
	typedef Bundle<ADDR> RegBundle;
	typedef Bundle<32> DataBundle;

	void Connect(const RegBundle& exmemRd, const Bundle<32>& exmemAluOut, const Wire& exmemRegWrite,
		const RegBundle& memwbRd, const Bundle<32>& memwbAluOut, const Wire& memwbRegWrite,
		const RegBundle& idexRs, const RegBundle& idexRt);
	void Update();
	
private:
	OrGateN<ADDR> ExMemRdNonZero;
	AndGate ExMemCanForward;
	OrGateN<ADDR> MemWbRdNonZero;
	AndGate MemWbCanForward;
	
	Matcher<ADDR> ExMemRdRs;
	Matcher<ADDR> ExMemRdRt;
	Matcher<ADDR> MemWbRdRs;
	Matcher<ADDR> MemWbRdRt;
};

