#include "W65C816.h"
#include "W65C816State.h"
#include "WriteAbsoluteAddressLow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteAbsoluteAddressLow::Init(void)
{
	CDataOperation::Init(false, true, true, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteAbsoluteAddressLow::Execute(CW65C816* pcCPU)
{
    pcCPU->GetState()->SetData(GetLowByte(pcCPU->GetState()->GetAddress()->GetOffset()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWriteAbsoluteAddressLow::Print(CChars* psz)
{
	psz->Append("Write(AAL)");
}

