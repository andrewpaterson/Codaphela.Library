#include "W65C816.h"
#include "WriteProgramCounterHigh.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteProgramCounterHigh::Init(void)
{
	CDataOperation::Init(false, true, true, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteProgramCounterHigh::Execute(CW65C816* pcCPU)
{
    pcCPU->WriteProgramCounterHigh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWriteProgramCounterHigh::IsInitialSide(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteProgramCounterHigh::Print(CChars* psz)
{
	psz->Append("Write(PCH)");
}

