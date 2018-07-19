#pragma once

#include "Bundle.h"
#include "Component.h"
#include "FullAdder.h"
#include "Comparator.h"
#include "MuxBundle.h"
#include "Multiplexer.h"


// BranchType:
//	0	BEQ 
//	1	BNE
//	2	BLEZ
//	3	BGTZ
//
class BranchControl : public Component
{
public:
	typedef Bundle<32> DataBundle;
	void Connect(const DataBundle& in1, const DataBundle& in2, const DataBundle& pc, const Bundle<16>& delta, 
		const Wire& enable, const Bundle<2> branchtype, const DataBundle& jumpaddr, const Wire& jumpr1);
	void Update();

	const Wire& BranchTaken() const { return branchAnd.Out(); }
	const DataBundle& NewPC() const { return addrMux.Out(); }

private:
	Comparator<32> comp;
	FullAdderN<32> adder;
	Multiplexer<4> branchTypeMux;
	MuxBundle<32, 2> jumpAddrMux;
	MuxBundle<32, 2> addrMux;
	AndGate branchAnd;
};

