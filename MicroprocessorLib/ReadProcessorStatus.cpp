#include "W65C816.h"
#include "ReadProcessorStatus.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadProcessorStatus::Init(void)
{
	CDataOperation::Init(false, true, true, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadProcessorStatus::Execute(CW65C816* pcCPU)
{
    pcCPU->ReadProcessorStatus();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadProcessorStatus::IsInitialSide(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadProcessorStatus::Print(CChars* psz)
{
	psz->Append("Read(P)");
}

