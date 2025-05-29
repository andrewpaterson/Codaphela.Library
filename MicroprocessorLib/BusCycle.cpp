#include "BusCycle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::Init(CAddressOffsetArray* papcAddressOffsets, COperationArray* papcOperations)
{
    size            uiNumOperations;
    size            i;
    COperation*     pcOperation;

    memcpy(&mapcAddressOffsets, papcAddressOffsets, sizeof(CAddressOffsetArray));
    free(papcAddressOffsets);

    memcpy(&mapcOperations, papcOperations, sizeof(COperationArray));
    free(papcOperations);

    miCycle = -1;

    Validate();

    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        if (pcOperation->IsData())
        {
            mpcDataOperation = (CDataOperation*)pcOperation;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::Kill(void)
{
    mpcDataOperation = NULL;

    mapcAddressOffsets.Kill();
    mapcOperations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::Validate(void)
{
    size            uiNumOperations;
    size            i;
    COperation*     pcOperation;
    size            uiDataBusOperation;

    uiDataBusOperation = 0;
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        if (pcOperation->IsData())
        {
            uiDataBusOperation++;
        }
    }

    if (uiDataBusOperation != 1)
    {
        LOG_ERROR("Exactly[1] data bus operation must be specified in a bus cycle.");
    }

    if (mapcAddressOffsets.NumElements() == 0)
    {
        LOG_ERROR("At least [1] address offset must be specified in a bus cycle.");
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::GetAddress(CAddress* pcDest, CW65C816* pcCPU)
{
    CalculateAddressFromAddressOffsets(pcDest, pcCPU, &mapcAddressOffsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDataOperation* CBusCycle::GetDataOperation(void)
{
    return mpcDataOperation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBusCycle::GetDone8(void)
{
    size            uiNumOperations;
    size            i;
    COperation* pcOperation;
    size            uiDone8;

    uiDone8 = 0;
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        uiDone8 += pcOperation->GetDone8();
    }
    return uiDone8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBusCycle::GetDone16(void)
{
    size            uiNumOperations;
    size            i;
    COperation* pcOperation;
    size            uiDone16;

    uiDone16 = 0;
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        uiDone16 += pcOperation->GetDone16();
    }
    return uiDone16;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBusCycle::MustExecute(CW65C816* pcCPU)
{
    size            uiNumOperations;
    size            i;
    COperation*     pcOperation;
    bool            bMustExecute;

    bMustExecute = true;
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        if (!pcOperation->MustExecute(pcCPU))
        {
            bMustExecute = false;
        }
    }
    return bMustExecute;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::ToAddressOffsetString(CChars* psz)
{
    size                uiNumOffsets;
    size                i;
    CAddressOffset* pcOffset;
    bool                bFirst;

    bFirst = true;
    uiNumOffsets = mapcAddressOffsets.NumElements();
    for (i = 0; i < uiNumOffsets; i++)
    {
        pcOffset = mapcAddressOffsets.GetPtr(i);
        if (!bFirst && !(pcOffset->IsConstantOffset()))
        {
            if (psz->GetChar(psz->Length() - 1) != ',')
            {
                psz->Append("+");
            }
        }
        pcOffset->Print(psz);

        bFirst = false;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::ToOperationString(CChars* psz)
{
    size            uiNumOperations;
    size            i;
    COperation* pcOperation;
    bool            bFirst;

    bFirst = true;
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        if (!bFirst)
        {
            psz->Append(" ");
        }
        pcOperation->Print(psz);
        bFirst = false;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::ToDataString(CChars* psz)
{
    mpcDataOperation->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBusCycle::IsFetchOpcode(void)
{
    return mpcDataOperation->IsFetchOpcode();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycle::SetCycle(int16 iCycle)
{
    miCycle = iCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CBusCycle::GetCycle(void)
{
    return miCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COperationArray* CBusCycle::GetOperations(void)
{
    return &mapcOperations;
}

