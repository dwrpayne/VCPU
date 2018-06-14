#include "BranchControl.h"

void BranchControl::Connect(const DataBundle & in1, const DataBundle & in2, const DataBundle & pc, const Bundle<16>& delta,
	const Wire & enable, const Bundle<2> branchtype, const DataBundle& jumpaddr, const Wire& jumpr1)
{
	comp.Connect(in1, in2);
	adder.Connect(pc, delta.SignExtend<32>(), Wire::OFF);
	branchTypeMux.Connect({ &comp.Equal(), &comp.NonEqual(), &comp.LessEqual(), &comp.Greater() },
		branchtype);
	branchAnd.Connect(branchTypeMux.Out(), enable);
	jumpAddrMux.Connect({ jumpaddr, in1}, jumpr1);
	addrMux.Connect({ jumpAddrMux.Out(), adder.Out() }, enable);
}

void BranchControl::Update()
{
	adder.Update();
	comp.Update();
	branchTypeMux.Update();
	branchAnd.Update();
	jumpAddrMux.Update();
	addrMux.Update();
}
