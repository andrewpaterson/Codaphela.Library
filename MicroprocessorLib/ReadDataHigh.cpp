#include "W65C816.h"
#include "ReadDataHigh.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadDataHigh::Init(NotMemoryLock bNotMemoryLock)
{
	CDataOperation::Init(false, true, bNotMemoryLock, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadDataHigh::Execute(CW65C816* pcCPU)
{
	pcCPU->SetDataHigh(pcCPU->GetIOData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CReadDataHigh::IsInitialSide(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CReadDataHigh::Print(CChars* psz)
{
	psz->Append("Read(DH)");
}

