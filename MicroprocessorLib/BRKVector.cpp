#include "W65C816.h"
#include "W65C816State.h"
#include "BRKVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CBRKVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->GetState()->IsEmulation())
    {
        return 0xfffe;
    }
    else
    {
        return 0xffe6;
    }
}

