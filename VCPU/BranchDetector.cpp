#include "BranchDetector.h"


void BranchDetector::Connect(const Wire& aluzero, const Wire& aluneg, const Bundle<2>& branchsel, const Wire& enable)
{
	// Handles 4 branch instructions  
	//	000100	beq rs rt	 rs - rt		alu.zero()
	//	000101	bneq rs rt	 rs - rt		NOT alu.zero()
	//	000110	blez rs 0	 rs - 0			alu.negative or alu.zero()
	//	000111	bgtz rs 0	 rs - 0			not (alu.negative or alu.zero())
	aluZeroInv.Connect(aluzero);
	aluNegOrZero.Connect(aluneg, aluzero);
	aluPos.Connect(aluNegOrZero.Out());
	branchTakenMux.Connect({ &aluzero, &aluZeroInv.Out(), &aluNegOrZero.Out(), &aluPos.Out() }, branchsel);
	branchTakenAnd.Connect(branchTakenMux.Out(), enable);
}


void BranchDetector::Update()
{
	aluZeroInv.Update();
	aluNegOrZero.Update();
	aluPos.Update();
	branchTakenMux.Update();
	branchTakenAnd.Update();
}
