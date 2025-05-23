#include "W65C816.h"
#include "W65C816State.h"
#include "DecrementStackPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecrementStackPointer::Init(void)
{
	COperation::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecrementStackPointer::Execute(CW65C816* pcCPU)
{
	pcCPU->GetState()->DecrementStackPointer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecrementStackPointer::Print(CChars* psz)
{
	psz->Append("SP--");
}

