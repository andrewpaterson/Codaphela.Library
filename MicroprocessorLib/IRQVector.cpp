#include "W65C816.h"
#include "W65C816State.h"
#include "IRQVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CIRQVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->GetState()->IsEmulation())
    {
        return 0xfffe;
    }
    else
    {
        return 0xffee;
    }
}

