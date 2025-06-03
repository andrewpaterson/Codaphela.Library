#include "Instruction.h"
#include "InstructionFactory.h"
#include "MetaW65C816.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C816::Init(MetaW65C816TickHigh fTickHigh, MetaW65C816TickLow fTickLow, void* pvContext)
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

	mfTickHigh = fTickHigh;
	mfTickLow = fTickLow;
	mpvContext = pvContext;
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
bool CMetaW65C816::TickInstruction(void)
{
	int		i;
	bool	bRunning;
	
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
		SetBank(mcPins.ReadData(&mcTimeline));
		SetX(mcPins.ReadMX(&mcTimeline));
		mfTickLow(this, mpvContext);

		mcPHI2.Invert();
		mcMPU.InputTransition(&mcTimeline);
	}

	for (;;)
	{
		mcPHI2.Invert();
		mcMPU.InputTransition(&mcTimeline);

		if (mcPHI2.IsHigh())
		{
			mfTickHigh(this, mpvContext);
		}

		if (mcPHI2.IsLow())
		{
			SetBank(mcPins.ReadData(&mcTimeline));
			SetX(mcPins.ReadMX(&mcTimeline));
			mfTickLow(this, mpvContext);

			if (mcMPU.IsStopped())
			{
				bRunning = false;
				break;
			}
			if (mcVDA.IsHigh() && mcVPA.IsHigh())
			{
				bRunning = true;
				break;
			}
		}

	}
	return bRunning;
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
bool CMetaW65C816::IsInstruction(CInstruction* pcExpectedInstruction)
{
	CInstruction* pcInstruction;

	pcInstruction = mcMPU.GetInstruction();
	return pcInstruction == pcExpectedInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C816::IsFetchOpcodeCycle(void)
{
	CDataOperation*		pcOperation;

	pcOperation = mcMPU.GetDataOperation();
	if (pcOperation != NULL)
	{
		if (pcOperation->IsFetchOpcode())
		{
			return true;
		}
	}
	return false;
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

	for (i = psz->Length() - 1; i >= 0; i--)
	{
		if (psz->GetChar(i) != ' ')
		{
			break;
		}
	}
	i = psz->Length() - i - 1;
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
void CMetaW65C816::SetBank(uint8 uiBank)
{
	muiBank = uiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CMetaW65C816::GetBank(void)
{
	return muiBank;
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

