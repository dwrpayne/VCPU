#pragma once

#include "Bundle.h"
#include "Component.h"
#include "FullAdder.h"
#include "Comparator.h"
#include "Extender.h"


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
		const Wire& enable, const Bundle<2> branchtype);
	void Update();

	const Wire& BranchTaken() const { return branchAnd.Out(); }
	const DataBundle& NewPC() const { return adder.Out(); }

private:
	Comparator<32> comp;
	FullAdderN<32> adder;
	Multiplexer<4> branchTypeMux;
	AndGate branchAnd;
};

