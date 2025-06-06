#include "W65C816.h"
#include "IRQVector.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRQVector::Init(void)
{
    CInterruptVector::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CIRQVector::GetAddress(CW65C816* pcCPU)
{
    if (pcCPU->IsEmulation())
    {
        return 0xfffe;
    }
    else
    {
        return 0xffee;
    }
}

