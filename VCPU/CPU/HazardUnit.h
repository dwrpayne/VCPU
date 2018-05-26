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

	const Bundle<2> AluRsMux() const { return { &ForwardExMemRs.Out(), &ForwardMemWbRs.Out() }; }
	const Bundle<2> AluRtMux() const { return { &ForwardExMemRt.Out(), &ForwardMemWbRt.Out() }; }
	const DataBundle& ForwardExMem() const { return ExMemData; }
	const DataBundle& ForwardMemWb() const { return MemWbData; }

private:
	DataBundle ExMemData;
	DataBundle MemWbData;

	OrGateN<ADDR> ExMemRdNonZero;
	OrGateN<ADDR> MemWbRdNonZero;
	
	Matcher<ADDR> ExMemRdRs;
	Matcher<ADDR> ExMemRdRt;
	Matcher<ADDR> MemWbRdRs;
	Matcher<ADDR> MemWbRdRt;

	AndGateN<3> ForwardExMemRs;
	AndGateN<3> ForwardExMemRt;
	Inverter NotForwardExMemRs;
	Inverter NotForwardExMemRt;

	AndGateN<4> ForwardMemWbRs;
	AndGateN<4> ForwardMemWbRt;
		
};
