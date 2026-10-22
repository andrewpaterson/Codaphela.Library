#include "Instruction.h"
#include "DataOperation.h"
#include "MetaW65C.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaW65C::InitTraces(void* pvContext)
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
void CMetaW65C::KillTraces(void)
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
void CMetaW65C::InputTransition(void)
{
	GetCore()->InputTransition(&mcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C::TickInstruction(void)
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
bool CMetaW65C::TestTickCycle(void)
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
bool CMetaW65C::IsInstruction(CInstruction* pcExpectedInstruction)
{
	CInstruction* pcInstruction;

	pcInstruction = GetCore()->GetInstruction();
	return pcInstruction == pcExpectedInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMetaW65C::IsFetchOpcodeCycle(void)
{
	CDataOperation*		pcOperation;

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
void CMetaW65C::SetBank(uint8 uiBank)
{
	muiBank = uiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CMetaW65C::GetBank(void)
{
	return muiBank;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMetaBus16* CMetaW65C::GetAddress(void) { return &mcAddress; }
CMetaBus8*  CMetaW65C::GetData(void)    { return &mcData; }
CMetaTrace* CMetaW65C::GetVPB(void)     { return &mcVPB; }
CMetaTrace* CMetaW65C::GetRDY(void)     { return &mcRDY; }
CMetaTrace* CMetaW65C::GetIRQB(void)	{ return &mcIRQB; }
CMetaTrace* CMetaW65C::GetNMIB(void)	{ return &mcNMIB; }
CMetaTrace* CMetaW65C::GetMLB(void)	    { return &mcMLB; }
CMetaTrace* CMetaW65C::GetRESB(void)	{ return &mcRESB; }
CMetaTrace* CMetaW65C::GetPHI2(void)	{ return &mcPHI2; }
CMetaTrace* CMetaW65C::GetBE(void)	    { return &mcBE; }
CMetaTrace* CMetaW65C::GetRWB(void)	    { return &mcRWB; }

