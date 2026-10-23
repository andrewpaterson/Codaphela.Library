#include "W65BusCycle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::Init(CW65AddressOffsetArray* papcAddressOffsets, CW65OperationArray* papcOperations)
{
    size            uiNumOperations;
    size            i;
    CW65Operation*     pcOperation;

    memcpy(&mapcAddressOffsets, papcAddressOffsets, sizeof(CW65AddressOffsetArray));
    free(papcAddressOffsets);

    memcpy(&mapcOperations, papcOperations, sizeof(CW65OperationArray));
    free(papcOperations);

    miCycle = -1;

    Validate();

    mapcConditions.Init();
    uiNumOperations = mapcOperations.NumElements();
    for (i = 0; i < uiNumOperations; i++)
    {
        pcOperation = mapcOperations.GetPtr(i);
        if (pcOperation->IsData())
        {
            mpcDataOperation = (CW65DataOperation*)pcOperation;
        }
        if (pcOperation->IsConditional())
        {
            mapcConditions.Add(pcOperation);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::Kill(void)
{
    mpcDataOperation = NULL;

    mapcConditions.Kill();
    mapcAddressOffsets.Kill();
    mapcOperations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::Validate(void)
{
    size            uiNumOperations;
    size            i;
    CW65Operation*     pcOperation;
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
void CW65BusCycle::GetAddress(CW65Address* pcDest, CW65CCore* pcCPU)
{
    CalculateAddressWrappingFromAddressOffsets(pcDest, pcCPU, &mapcAddressOffsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65DataOperation* CW65BusCycle::GetDataOperation(void)
{
    return mpcDataOperation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CW65BusCycle::GetDone8(void)
{
    size            uiNumOperations;
    size            i;
    CW65Operation* pcOperation;
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
size CW65BusCycle::GetDone16(void)
{
    size            uiNumOperations;
    size            i;
    CW65Operation* pcOperation;
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
bool CW65BusCycle::MustExecute(CW65CCore* pcCPU)
{
    size            uiNumConditions;
    size            i;
    CW65Operation*     pcOperation;

    uiNumConditions = mapcConditions.NumElements();
    for (i = 0; i < uiNumConditions; i++)
    {
        pcOperation = mapcConditions.GetPtr(i);
        if (!pcOperation->MustExecute(pcCPU))
        {
            return false;
        }
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::ToAddressOffsetString(CChars* psz)
{
    mapcAddressOffsets.Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::ToOperationString(CChars* psz)
{
    size            uiNumOperations;
    size            i;
    CW65Operation* pcOperation;
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
void CW65BusCycle::ToDataString(CChars* psz)
{
    mpcDataOperation->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65BusCycle::IsFetchOpcode(void)
{
    return mpcDataOperation->IsFetchOpcode();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycle::SetCycle(int16 iCycle)
{
    miCycle = iCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CW65BusCycle::GetCycle(void)
{
    return miCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65OperationArray* CW65BusCycle::GetOperations(void)
{
    return &mapcOperations;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65BusCycle::IsProgramAddress(void)
{
    size            uiNumOffsets;
    size            i;
    CW65AddressOffset* pcOffset;

    uiNumOffsets = mapcAddressOffsets.NumElements();
    for (i = 0; i < uiNumOffsets; i++)
    {
        pcOffset = mapcAddressOffsets.GetPtr(i);
        if (pcOffset->IsProgramCounter())
        {
            return true;
        }
    }
    return false;
}

