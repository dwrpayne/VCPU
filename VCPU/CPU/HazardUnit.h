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

	const DataBundle& ForwardDataRs() const { return ForwardRsDataMux.Out(); }
	const DataBundle& ForwardDataRt() const { return ForwardRtDataMux.Out(); }
	const Wire& DoForwardRs() const { return DoForwardRsOr.Out(); }
	const Wire& DoForwardRt() const { return DoForwardRtOr.Out(); }

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

	MuxBundle<32, 2> ForwardRsDataMux;
	MuxBundle<32, 2> ForwardRtDataMux;

	OrGate DoForwardRsOr;
	OrGate DoForwardRtOr;
		
	friend class Debugger;
};
