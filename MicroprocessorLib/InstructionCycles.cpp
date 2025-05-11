#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "InstructionCycles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionCycles::Init(EAddressingMode eAddressingMode, CBusCycleArray* papcCycles)
{
    CBusCycle*  pcBusCycle;
    size        uiNumCycles;
    size        i;

    memcpy(&mapcCycles, papcCycles, sizeof(CBusCycleArray));
    free(papcCycles);

    meAddressingMode = eAddressingMode;

    uiNumCycles = mapcCycles.NumElements();
    for (i = 0; i < uiNumCycles; i++)
    {
        pcBusCycle = mapcCycles.GetPtr(i);
        pcBusCycle->SetCycle((int16)(i + 1));
    }

    Validate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionCycles::Kill(void)
{
    mapcCycles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionCycles::Validate(void)
{
    ValidateDoneOperation();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionCycles::ValidateDoneOperation(void)
{
    CBusCycle* pcBusCycle;
    size        uiNumCycles;
    size        i;
    size        uiDone8;
    size        uiDone16;

    uiDone8 = 0;
    uiDone16 = 0;

    uiNumCycles = mapcCycles.NumElements();
    for (i = 0; i < uiNumCycles; i++)
    {
        pcBusCycle = mapcCycles.GetPtr(i);
        uiDone8 += pcBusCycle->GetDone8();
        uiDone16 += pcBusCycle->GetDone16();
    }

    if (uiDone8 != 1 && uiDone16 != 1)
    {
        LOG_ERROR("Exactly [1] 8 bit and [1] 16 bit done  operation must be specified in an Instruction cycle.");
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EAddressingMode CInstructionCycles::GetAddressingMode(void)
{
    return meAddressingMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBusCycle* CInstructionCycles::GetBusCycle(int16 miIndex)
{
    if ((miIndex >= 0) && (miIndex < (int16)mapcCycles.NumElements()))
    {
        return mapcCycles.GetPtr(miIndex);
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInstructionCycles::Size(void)
{
    return mapcCycles.NumElements();
}

