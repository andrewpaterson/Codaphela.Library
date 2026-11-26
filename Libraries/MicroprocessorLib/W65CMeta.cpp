#include "W65Instruction.h"
#include "W65DataOperation.h"
#include "W65CMeta.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65CMeta::InitTraces(void* pvContext)
{
	mcAddress.Init();
	mcData.Init();
	mcPHI2.Init(false);
	mcRESB.Init(false);
	mcRWB.Init();
	mcVPB.Init();
	mcIRQB.Init(true);
	mcNMIB.Init(true);
	mcMLB.Init();
	mcBE.Init(true);
	mcRDY.Init(true);

	mpvContext = pvContext;
	muiBank = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65CMeta::KillTraces(void)
{
	mcPHI2.Init(true);
	mcRESB.Init(false);
	mcRWB.Kill();
	mcVPB.Kill();
	mcIRQB.Kill();
	mcNMIB.Kill();
	mcMLB.Kill();
	mcBE.Kill();
	mcRDY.Kill();
	mcData.Kill();
	mcAddress.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65CMeta::InputTransition(void)
{
	GetCore()->InputTransition(&mcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65CMeta::TickInstruction(void)
{
	int		i;
	bool	bRunning;

	if (mcRESB.IsLow())
	{
		for (i = 0; i < 4; i++)
		{
			mcPHI2.Invert();
			InputTransition();
		}
		mcRESB.Set(true);

		mcPHI2.Invert();
		InputTransition();
		LatchPhi2Low();
		CallTickLow();

		mcPHI2.Invert();
		InputTransition();
	}

	for (;;)
	{
		mcPHI2.Invert();
		InputTransition();

		if (mcPHI2.IsHigh())
		{
			CallTickHigh();
		}

		if (mcPHI2.IsLow())
		{
			LatchPhi2Low();
			CallTickLow();

			if (GetCore()->IsStopped())
			{
				bRunning = false;
				break;
			}
			if (IsOpcodeFetchSignalled() || GetCore()->IsWaiting())
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
bool CW65CMeta::TestTickCycle(void)
{
	//Only for tests.  Runs one PHI2 cycle, high then low, without handling reset.
	//Returns true if the next cycle is an opcode fetch.
	int		i;

	for (i = 0; i < 2; i++)
	{
		mcPHI2.Invert();
		InputTransition();

		if (mcPHI2.IsHigh())
		{
			CallTickHigh();
		}
		else
		{
			LatchPhi2Low();
			CallTickLow();
		}
	}
	return IsOpcodeFetchSignalled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65CMeta::IsInstruction(CW65Instruction* pcExpectedInstruction)
{
	CW65Instruction* pcInstruction;

	pcInstruction = GetCore()->GetInstruction();
	return pcInstruction == pcExpectedInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65CMeta::IsFetchOpcodeCycle(void)
{
	CW65DataOperation*		pcOperation;

	pcOperation = GetCore()->GetDataOperation();
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
void CW65CMeta::SetBank(uint8 uiBank)
{
	muiBank = uiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CW65CMeta::GetBank(void) const
{
	return muiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMetaBus16* CW65CMeta::GetAddress(void) { return &mcAddress; }
CMetaBus8*  CW65CMeta::GetData(void)    { return &mcData; }
const CMetaTrace* CW65CMeta::GetVPB(void) const     { return &mcVPB; }
const CMetaTrace* CW65CMeta::GetRDY(void) const     { return &mcRDY; }
CMetaTrace* CW65CMeta::GetIRQB(void)	{ return &mcIRQB; }
CMetaTrace* CW65CMeta::GetNMIB(void)	{ return &mcNMIB; }
const CMetaTrace* CW65CMeta::GetMLB(void) const	    { return &mcMLB; }
CMetaTrace* CW65CMeta::GetRESB(void)	{ return &mcRESB; }
const CMetaTrace* CW65CMeta::GetPHI2(void) const	{ return &mcPHI2; }
const CMetaTrace* CW65CMeta::GetBE(void) const	    { return &mcBE; }
const CMetaTrace* CW65CMeta::GetRWB(void) const	    { return &mcRWB; }

