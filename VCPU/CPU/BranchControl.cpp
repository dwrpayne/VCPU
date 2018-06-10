#include "BranchControl.h"

void BranchControl::Connect(const DataBundle & in1, const DataBundle & in2, const DataBundle & pc, const Bundle<16>& delta,
	const Wire & enable, const Bundle<2> branchtype)
{
	comp.Connect(in1, in2);
	adder.Connect(pc, delta.SignExtend<32>(), Wire::OFF);
	branchTypeMux.Connect({ &comp.Equal(), &comp.NonEqual(), &comp.LessEqual(), &comp.Greater() },
		branchtype);
	branchAnd.Connect(branchTypeMux.Out(), enable);
}

void BranchControl::Update()
{
	adder.Update();
	comp.Update();
	branchTypeMux.Update();
	branchAnd.Update();
}
