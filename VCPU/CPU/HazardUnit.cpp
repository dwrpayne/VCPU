#include "HazardUnit.h"


void HazardUnit::Connect(const RegBundle& exmemRd, const Bundle<32>& exmemAluOut, const Wire& exmemRegWrite,
	const RegBundle& memwbRd, const Bundle<32>& memwbAluOut, const Wire& memwbRegWrite,
	const RegBundle& idexRs, const RegBundle& idexRt)
{
	ExMemData = exmemAluOut;
	MemWbData = memwbAluOut;
	ExMemRdNonZero.Connect(exmemRd);
	MemWbRdNonZero.Connect(memwbRd);

	ExMemRdRs.Connect(exmemRd, idexRs);
	ExMemRdRt.Connect(exmemRd, idexRt);
	MemWbRdRs.Connect(memwbRd, idexRs);
	MemWbRdRt.Connect(memwbRd, idexRt);

	ForwardExMemRs.Connect({ &ExMemRdNonZero.Out(), &exmemRegWrite, &ExMemRdRs.Out() });
	ForwardExMemRt.Connect({ &ExMemRdNonZero.Out(), &exmemRegWrite, &ExMemRdRt.Out() });

	NotForwardExMemRs.Connect(ForwardExMemRs.Out());
	NotForwardExMemRt.Connect(ForwardExMemRt.Out());

	ForwardMemWbRs.Connect({ &MemWbRdNonZero.Out(), &memwbRegWrite,
		&MemWbRdRs.Out(), &NotForwardExMemRs.Out() });

	ForwardMemWbRt.Connect({ &MemWbRdNonZero.Out(), &memwbRegWrite,
		&MemWbRdRt.Out(), &NotForwardExMemRt.Out() });
}

void HazardUnit::Update()
{
	ExMemRdNonZero.Update();
	MemWbRdNonZero.Update();

	ExMemRdRs.Update();
	ExMemRdRt.Update();
	MemWbRdRs.Update();
	MemWbRdRt.Update();

	ForwardExMemRs.Update();
	ForwardExMemRt.Update();
	NotForwardExMemRs.Update();
	NotForwardExMemRt.Update();
	ForwardMemWbRs.Update();
	ForwardMemWbRt.Update();
	
}
