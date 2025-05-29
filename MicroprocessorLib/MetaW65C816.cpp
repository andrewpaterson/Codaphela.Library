#include "MetaW65C816.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Init(void)
{
	mcAddress.Init();
	mcData.Init();
	mcPHI2.Init(false);
	mcRESB.Init(false);
	mcRWB.Init();
	mcVPA.Init();
	mcVDA.Init();
	mcVPB.Init();
	mcABORTB.Init(true);
	mcIRQB.Init(true);
	mcNMIB.Init(true);
	mcMLB.Init();
	mcMX.Init();
	mcBE.Init(true);
	mcE.Init();
	mcRDY.Init(true);

	mcPins.Init(&mcAddress, &mcData, &mcPHI2, &mcRESB, &mcRWB);
	mcPins.SetImportantTraces(&mcVPA, &mcVDA, &mcVPB);
	mcPins.SetInterruptTraces(&mcABORTB, &mcIRQB, &mcNMIB);
	mcPins.SetOtherTraces(&mcMLB, &mcMX, &mcBE, &mcE, &mcRDY);

	mcMPU.Init(&mcPins);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Kill(void)
{
	mcMPU.Kill();
	mcPins.Kill();

	mcPHI2.Init(true);
	mcRESB.Init(false);
	mcRWB.Kill();
	mcVPA.Kill();
	mcVDA.Kill();
	mcVPB.Kill();
	mcABORTB.Kill();
	mcIRQB.Kill();
	mcNMIB.Kill();
	mcMLB.Kill();
	mcMX.Kill();
	mcBE.Kill();
	mcE.Kill();
	mcRDY.Kill();
	mcData.Kill();
	mcAddress.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::TickInstruction(void)
{
	int i = 0;
	if (mcRESB.IsLow())
	{
		for (i = 0; i < 4; i++)
		{
			mcPHI2.Invert();
			mcMPU.InputTransition(&mcTimeline);
		}
		mcRESB.Set(true);

		mcPHI2.Invert();
		mcMPU.InputTransition(&mcTimeline);
		Dump();

		mcPHI2.Invert();
		mcMPU.InputTransition(&mcTimeline);
	}

	for (;;)
	{
		mcPHI2.Invert();
		mcMPU.InputTransition(&mcTimeline);

		if (mcPHI2.IsHigh())
		{
		}

		if (mcPHI2.IsLow())
		{
			Dump();
			if (mcVDA.IsHigh() && mcVPA.IsHigh())
			{
				return;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Print(CChars* psz)
{
	mcMPU.GetOpcodeMnemonicString(psz);
	psz->Append(": (");
	mcMPU.GetCycleString(psz);
	psz->Append(") ");
	mcMPU.GetCycleOperationString(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65C816*   CMetaW65C816::GetMPU(void)     { return &mcMPU; }
CMetaBus16* CMetaW65C816::GetAddress(void) { return &mcAddress; }
CMetaBus8*  CMetaW65C816::GetData(void)    { return &mcData; }
CMetaTrace* CMetaW65C816::GetVPB(void)     { return &mcVPB; }
CMetaTrace* CMetaW65C816::GetRDY(void)     { return &mcRDY; }
CMetaTrace* CMetaW65C816::GetABORTB(void)  { return &mcABORTB; }
CMetaTrace* CMetaW65C816::GetIRQB(void)	   { return &mcIRQB; }
CMetaTrace* CMetaW65C816::GetNMIB(void)	   { return &mcNMIB; }
CMetaTrace* CMetaW65C816::GetMLB(void)	   { return &mcMLB; }
CMetaTrace* CMetaW65C816::GetVPA(void)	   { return &mcVPA; }
CMetaTrace* CMetaW65C816::GetVDA(void)	   { return &mcVDA; }
CMetaTrace* CMetaW65C816::GetRESB(void)	   { return &mcRESB; }
CMetaTrace* CMetaW65C816::GetMX(void)	   { return &mcMX; }
CMetaTrace* CMetaW65C816::GetPHI2(void)	   { return &mcPHI2; }
CMetaTrace* CMetaW65C816::GetBE(void)	   { return &mcBE; }
CMetaTrace* CMetaW65C816::GetE(void)	   { return &mcE; }
CMetaTrace* CMetaW65C816::GetRWB(void)	   { return &mcRWB; }

