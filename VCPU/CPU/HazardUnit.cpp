#include "HazardUnit.h"


void HazardUnit::Connect(const RegBundle& exmemRd, const Bundle<32>& exmemAluOut, const Wire& exmemRegWrite,
	const RegBundle& memwbRd, const Bundle<32>& memwbAluOut, const Wire& memwbRegWrite,
	const RegBundle& idexRs, const RegBundle& idexRt)
{
	ExMemRdNonZero.Connect(exmemRd);
	MemWbRdNonZero.Connect(exmemRd);
	ExMemCanForward.Connect(&ExMemRdNonZero.Out(), exmemRegWrite);
	MemWbCanForward.Connect(MemWbRdNonZero.Out(), memwbRegWrite);

	ExMemRdRs.Connect(exmemRd, idexRs);
	ExMemRdRt.Connect(exmemRd, idexRt);
	MemWbRdRs.Connect(memwbRd, idexRs);
	MemWbRdRt.Connect(memwbRd, idexRt);

}

void HazardUnit::Update()
{
	ExMemRdNonZero.Update();
	MemWbRdNonZero.Update();
	ExMemCanForward.Update();
	MemWbCanForward.Update();

	ExMemRdRs.Update();
	ExMemRdRt.Update();
	MemWbRdRs.Update();
	MemWbRdRt.Update();



}
