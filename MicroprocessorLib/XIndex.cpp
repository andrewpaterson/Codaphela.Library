#include "W65C816.h"
#include "W65C816State.h"
#include "XIndex.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXIndex::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXIndex::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CXIndex::GetOffset(CW65C816* pcCPU)
{
    return pcCPU->GetState()->GetX();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXIndex::Print(CChars* psz)
{
    psz->Append("X");
}

