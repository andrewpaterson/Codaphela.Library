#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "W65InstructionCycles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65InstructionCycles::Init(EAddressingMode eAddressingMode, CW65BusCycleArray* papcCycles)
{
    CW65BusCycle*  pcBusCycle;
    size        uiNumCycles;
    size        i;

    memcpy(&mapcCycles, papcCycles, sizeof(CW65BusCycleArray));
    free(papcCycles);

    meAddressingMode = eAddressingMode;

    uiNumCycles = mapcCycles.NumElements();
    for (i = 0; i < uiNumCycles; i++)
    {
        pcBusCycle = mapcCycles.GetPtr(i);
        pcBusCycle->SetCycle((int16)(i + 1));
    }

    ValidateComponent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65InstructionCycles::Kill(void)
{
    mapcCycles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65InstructionCycles::ValidateComponent(void)
{
    ValidateDoneOperation();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65InstructionCycles::ValidateDoneOperation(void)
{
    CW65BusCycle* pcBusCycle;
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
EAddressingMode CW65InstructionCycles::GetAddressingMode(void)
{
    return meAddressingMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65BusCycle* CW65InstructionCycles::GetBusCycle(int16 miIndex)
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
size CW65InstructionCycles::Size(void)
{
    return mapcCycles.NumElements();
}

