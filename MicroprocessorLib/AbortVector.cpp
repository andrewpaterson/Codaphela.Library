#include "W65C816.h"
#include "AbortVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAbortVector::Init(void)
{
    CInterruptVector::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CAbortVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->IsEmulation())
    {
        return 0xfff8;
    }
    else
    {
        return 0xffe8;
    }
}

