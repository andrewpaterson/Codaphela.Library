#include "W65C816.h"
#include "W65C816State.h"
#include "NMIVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CNMIVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->GetState()->IsEmulation())
    {
        return 0xfffa;
    }
    else
    {
        return 0xffea;
    }
}

