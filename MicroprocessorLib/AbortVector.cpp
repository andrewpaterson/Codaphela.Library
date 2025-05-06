#include "W65C816.h"
#include "W65C816State.h"
#include "AbortVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CAbortVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->GetState()->IsEmulation())
    {
        return 0xfff8;
    }
    else
    {
        return 0xffe8;
    }
}

