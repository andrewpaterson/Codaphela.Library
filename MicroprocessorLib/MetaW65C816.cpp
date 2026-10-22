#include "Instruction.h"
#include "InstructionFactory.h"
#include "MetaW65C816.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Init(MetaW65C816TickHigh fTickHigh, MetaW65C816TickLow fTickLow, void* pvContext)
{
	InitTraces(pvContext);
	mcVPA.Init();
	mcVDA.Init();
	mcABORTB.Init(true);
	mcMX.Init();
	mcE.Init();

	mcPins.Init(&mcAddress, &mcData, &mcPHI2, &mcRESB, &mcRWB);
	mcPins.SetImportantTraces(&mcVPA, &mcVDA, &mcVPB);
	mcPins.SetInterruptTraces(&mcABORTB, &mcIRQB, &mcNMIB);
	mcPins.SetOtherTraces(&mcMLB, &mcMX, &mcBE, &mcE, &mcRDY);

	mcMPU.Init(&mcPins);

	mfTickHigh = fTickHigh;
	mfTickLow = fTickLow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Kill(void)
{
	mcMPU.Kill();
	mcPins.Kill();

	mcVPA.Kill();
	mcVDA.Kill();
	mcABORTB.Kill();
	mcMX.Kill();
	mcE.Kill();
	KillTraces();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C816::IsResetInstruction(void)
{
	return IsInstruction(CInstructionFactory::GetInstance()->GetReset());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C816::IsStopInstruction(void)
{
	return IsInstruction(CInstructionFactory::GetInstance()->GetInstruction(STP_Implied));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Print(CChars* psz)
{
	Print(psz, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Print(CChars* psz, bool bMnemonic, bool bCycle, bool bOperation, bool bA, bool bX, bool bY, bool bPC, bool bS, bool bDP, bool bDB, bool bP, bool bAddressBus, bool bDataBus, bool bBank, bool bVPB, bool bRDY, bool bMLB, bool bVPA, bool bVDA, bool bWM, bool bWX, bool bE, bool bRWB, bool bAddressOperation, bool bDataOperation, bool bDescription)
{
	CChars	sz;
	int32	i;

	if (bMnemonic)
	{
		mcMPU.GetOpcodeMnemonicString(psz);
		psz->Append(": ");
	}
	
	if (bCycle)
	{
		psz->Append("(");
		mcMPU.GetCycleString(psz);
		psz->Append(")  ");
	}

	if (bOperation)
	{
		sz.Init();
		mcMPU.GetCycleOperationString(&sz);
		psz->LeftAlign(sz, ' ', 16);
		sz.Kill();
	}

	if (bA)
	{
		psz->Append("A.");
		mcMPU.GetAccumulatorValueHex(psz, false);
		psz->Append("  ");
	}
	if (bX)
	{
		psz->Append("X.");
		mcMPU.GetXValueHex(psz, false);
		psz->Append("  ");
	}
	if (bY)
	{
		psz->Append("Y.");
		mcMPU.GetYValueHex(psz, false);
		psz->Append("  ");
	}
	if (bPC)
	{
		psz->Append("PC.");
		mcMPU.GetProgramCounterValueHex(psz, false);
		psz->Append("  ");
	}
	if (bS)
	{
		psz->Append("S.");
		mcMPU.GetStackValueHex(psz, false);
		psz->Append("  ");
	}
	if (bDP)
	{
		psz->Append("DP.");
		mcMPU.GetDirectPageValueHex(psz, false);
		psz->Append("  ");
	}
	if (bDB)
	{
		psz->Append("DB.");
		mcMPU.GetDataBankValueHex(psz, false);
		psz->Append("  ");
	}
	if (bP)
	{
		psz->Append("  P(");
		mcMPU.GetStatusString(psz);
		psz->Append(")  ");
	}

	psz->Append("  ");

	if (bBank && bAddressBus)
	{
		psz->Append("Addr.");
		psz->AppendHexHiLo(&muiBank, 1);
		psz->Append(":");
		mcAddress.Print(psz, false);
		psz->Append("  ");
	}
	else if (bAddressBus)
	{
		psz->Append("Addr.");
		mcAddress.Print(psz, false);
		psz->Append("  ");
	}
	else if (bBank)
	{
		psz->Append("Bank.");
		psz->AppendHexHiLo(&muiBank, 1);
		psz->Append("  ");
	}
	if (bDataBus)
	{
		psz->Append("Data.");
		mcData.Print(psz, false);
		psz->Append("  ");
	}
	if (bRWB)
	{
		psz->Append("RWB");
		mcRWB.Print(psz);
		psz->Append(" ");
	}
	if (bVPA)
	{
		psz->Append("VPA");
		mcVPA.Print(psz);
		psz->Append(" ");
	}
	if (bVDA)
	{
		psz->Append("VDA");
		mcVDA.Print(psz);
		psz->Append(" ");
	}
	if (bVPB)
	{
		psz->Append("VPB");
		mcVPB.Print(psz);
		psz->Append(" ");
	}
	if (bE)
	{
		psz->Append("E");
		mcE.Print(psz);
		psz->Append(" ");
	}
	if (bRDY)
	{
		psz->Append("RDY");
		mcRDY.Print(psz);
		psz->Append(" ");
	}
	if (bMLB)
	{
		psz->Append("MLB");
		mcMLB.Print(psz);
		psz->Append(" ");
	}
	if (bWM)
	{
		psz->Append("M");
		mcMX.Print(psz);
		psz->Append(" ");
	}
	if (bWX)
	{
		psz->Append("X");
		msX.Print(psz);
		psz->Append(" ");
	}

	if (bAddressOperation)
	{
		psz->Append("   ");
		sz.Init();
		mcMPU.ToAddressOperationString(&sz);
		psz->LeftAlign(sz, ' ', 11);
		sz.Kill();
		
	}
	if (bDataOperation)
	{
		psz->Append("   ");
		sz.Init();
		mcMPU.ToDataOperationString(&sz);
		psz->LeftAlign(sz, ' ', 60);
		sz.Kill();
	}
	if (bDescription)
	{
		psz->Append("   ");
		mcMPU.ToInstructionDescription(psz);
		psz->Append(".");
	}

	for (i = (int32)psz->Length() - 1; i >= 0; i--)
	{
		if (psz->GetChar(i) != ' ')
		{
			break;
		}
	}
	i = (int32)psz->Length() - i - 1;
	psz->RemoveFromEnd(i);
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
void CMetaW65C816::SetX(STraceValue sX)
{
	msX = sX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STraceValue CMetaW65C816::GetX(void)
{
	return msX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C816::IsOpcodeFetchSignalled(void)
{
	return mcVDA.IsHigh() && mcVPA.IsHigh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::LatchPhi2Low(void)
{
	SetBank(mcPins.ReadData(&mcTimeline));
	SetX(mcPins.ReadMX(&mcTimeline));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::CallTickHigh(void)
{
	mfTickHigh(this, mpvContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::CallTickLow(void)
{
	mfTickLow(this, mpvContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65C816*   CMetaW65C816::GetMPU(void)     { return &mcMPU; }
CW65CCore*  CMetaW65C816::GetCore(void)    { return &mcMPU; }
CMetaTrace* CMetaW65C816::GetABORTB(void)  { return &mcABORTB; }
CMetaTrace* CMetaW65C816::GetVPA(void)	   { return &mcVPA; }
CMetaTrace* CMetaW65C816::GetVDA(void)	   { return &mcVDA; }
CMetaTrace* CMetaW65C816::GetMX(void)	   { return &mcMX; }
CMetaTrace* CMetaW65C816::GetE(void)	   { return &mcE; }

