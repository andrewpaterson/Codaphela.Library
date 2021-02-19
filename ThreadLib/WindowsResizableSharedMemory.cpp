#include "BaseLib/Logger.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/WindowsError.h"
#include "ResizableSharedMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::Init(char* szMemoryName, char* szDebugIdentifier)
{
    BOOL    bResult;

    bResult = InitCoordinator(szMemoryName);
    mcHoldingMappedFile.Init();
    mpsHoldingDescriptor = NULL;

    mszSharedName.Init(szMemoryName);
    if (mszSharedName.Length() > 60)
    {
        mszSharedName.Remove(61, mszSharedName.Length());
    }

    if (szDebugIdentifier)
    {
        mszDebugIdentifier.Init(szDebugIdentifier);
    }
    else
    {
        mszDebugIdentifier.Init();
    }

    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::InitCoordinator(char* szMemoryName)
{
    CChars  sz;
    BOOL    bResult;

    sz.Init(szMemoryName);
    sz.Append(":C");
    bResult = mcCoordinator.Init(sz.Text());
    sz.Kill();

    //mpcCoordinatorMappedFile, mcMappedFile, mpvMemory and mpsDescriptor all move together
    mpcCoordinatorMappedFile = NULL;

    mcMappedFile.Init();

    mpvMemory = NULL;
    mpsDescriptor = NULL;

    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::ReinitCoordinator(void)
{
    mpcCoordinatorMappedFile = NULL;
    mcMappedFile.Close();
    mcMappedFile.Reinit();
    mpsDescriptor = NULL;
    mpvMemory = NULL;

    return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::Kill(void)
{
    if (mpsHoldingDescriptor)
    {
        UnmapViewOfFile(mpsHoldingDescriptor);
        mcHoldingMappedFile.Close();
        mpsHoldingDescriptor = NULL;
    }
    mcHoldingMappedFile.Kill();

    mszDebugIdentifier.Kill();
    mszSharedName.Kill();

    mpvMemory = NULL;
    mpsDescriptor = NULL;

    mcCoordinator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::Create(size_t uiSize)
{
    TouchHolding();

    if (mcMappedFile.IsNamed())
    {
        return gcLogger.Error2(__METHOD__, " Could not create map file.  Already exists [", mcMappedFile.GetName(), "].", NULL);
    }

    mpcCoordinatorMappedFile = mcCoordinator.Add(mszSharedName.Text());
    if (!mpcCoordinatorMappedFile)
    {
        return gcLogger.Error2(__METHOD__, " Could not create map file.  Previously created must be invalidated first [", mcMappedFile.GetName(), "].", NULL);
    }

    mcMappedFile.Reinit(mpcCoordinatorMappedFile->mszName);

    SSharedMemoryResult sResult = mcMappedFile.Create(uiSize);
    if (sResult.IsFailed())
    {
        return ReinitCoordinator();
    }

    if (Map(uiSize))
    {
        StrCpySafe(mpsDescriptor->szName, mszSharedName.Text(), 64);
        return TRUE;
    }
    else
    {
        return TRUE;
    }

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::Connect(void)
{
    TouchHolding();

    if (mcMappedFile.IsNamed())
    {
        return gcLogger.Error2(__METHOD__, " Could not connect to map file.  Already connected [", mcMappedFile.GetName(), "].", NULL);
    }

    mpcCoordinatorMappedFile = mcCoordinator.GetCurrent();
    if (mpcCoordinatorMappedFile)
    {
        if (StrEmpty(mpcCoordinatorMappedFile->mszName))
        {
            return gcLogger.Error2(__METHOD__, " Could not connect to map file.  Co-ordinator name is empty.", NULL);
        }

        mcMappedFile.Reinit(mpcCoordinatorMappedFile->mszName);

        SSharedMemoryResult sResult = mcMappedFile.Open();
        if (sResult.IsFailed())
        {
            
            gcLogger.Error2(__METHOD__, " Could not connect to map file.  No map file exists [", mpcCoordinatorMappedFile->mszName, "].", NULL);
            return ReinitCoordinator();
        }

        return Map(sResult.GetSize());
    }
    else
    {
        gcLogger.Error2(__METHOD__, " Could not connect to map file.  No map file exists.", NULL);
        return ReinitCoordinator();
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryMap CResizableSharedMemory::Touch(void)
{
    BOOL    bResult;

    if (mpsDescriptor)
    {
        if (mpsDescriptor->iInvalid)
        {
            Close();
            bResult = Connect();
            
            if (bResult)
            {
                return SSharedMemoryMap(mpvMemory, (size_t)mpsDescriptor->uiSize);
            }
            return SSharedMemoryMap();
        }
        else 
        {
            //Test the name and the map count.
            return SSharedMemoryMap(mpvMemory, (size_t)mpsDescriptor->uiSize);
        }
    }
    else
    {
        Close();
        if (Connect())
        {
            return SSharedMemoryMap(mpvMemory, (size_t)mpsDescriptor->uiSize);
        }
        return SSharedMemoryMap();
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::Map(size_t uiSize)
{
    mpsDescriptor = mcMappedFile.Map(uiSize);
    if (mpsDescriptor == NULL)
    {
        gcLogger.Error2(__METHOD__, " Could not map view of file [", mpcCoordinatorMappedFile->mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return ReinitCoordinator();
    }

    mpsDescriptor->iMapCount++;
    mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CResizableSharedMemory::Remap(size_t uiSize)
{
    uint64                      uiOldSize;

    if (mpsDescriptor)
    {
        uiOldSize = mpsDescriptor->uiSize;
        mpsDescriptor->iMapCount--;
        UnmapViewOfFile(mpsDescriptor);
     
        mpsDescriptor = mcMappedFile.Map(uiSize);
        if (mpsDescriptor == NULL)
        {
            mpvMemory = NULL;
            mpsDescriptor = mcMappedFile.Map((size_t)uiOldSize);
            if (mpsDescriptor == NULL)
            {
                gcLogger.Error2(__METHOD__, " Could not re-map view of file [", mpcCoordinatorMappedFile->mszName, "] back to original size.  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
                return ReinitCoordinator();
            }
            mpsDescriptor->iMapCount++;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
            return FALSE;
        }
        else
        {
            mpsDescriptor->iMapCount++;
            mpsDescriptor->uiSize = uiSize;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::HoldValid(void)
{
    mcHoldingMappedFile.Reinit(mpcCoordinatorMappedFile->mszName);
    mcHoldingMappedFile.Open();
    mpsHoldingDescriptor = mcHoldingMappedFile.Map(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::TouchHolding(void)
{
    if (mpsHoldingDescriptor)
    {
        if (mpsHoldingDescriptor->iInvalid == SHARED_MEMORY_INVALID)
        {
            UnmapViewOfFile(mpsHoldingDescriptor);
            mcHoldingMappedFile.Close();
            mpsHoldingDescriptor = NULL;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CResizableSharedMemory::Close(void)
{
    int     iStillMapped;

    TouchHolding();

    if (mpsDescriptor)
    {
        mpsDescriptor->iMapCount--;
        iStillMapped = mpsDescriptor->iMapCount;
        if (mpsDescriptor->iMapCount == 0)
        {
            if (mpsDescriptor->iInvalid == SHARED_MEMORY_VALID)
            {
                HoldValid();
            }
        }

        UnmapViewOfFile(mpsDescriptor);
        mcMappedFile.Close();
    }
    else
    {
        iStillMapped = 0;
    }
    ReinitCoordinator();

    return iStillMapped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CResizableSharedMemory::GetSize(void)
{
    return (size_t)mpsDescriptor->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryMap CResizableSharedMemory::Resize(size_t uiSize)
{
    BOOL                        bResult;
    uint64                      uiOldSize;
    int                         iStillMapped;
    void*                       pvMemory;

    bResult = Remap(uiSize);
    if (!bResult)
    { 
        mcCoordinator.Invalidate();
        mpsDescriptor->iInvalid = SHARED_MEMORY_INVALID;
        uiOldSize = mpsDescriptor->uiSize;
        pvMemory = malloc((size_t)uiOldSize);
        memcpy(pvMemory, mpvMemory, (size_t)uiOldSize);

        iStillMapped = Close();
        bResult = Create(uiSize);
        if (bResult)
        {
            memcpy(mpvMemory, pvMemory, (size_t)uiOldSize);
            free(pvMemory);
         
            return SSharedMemoryMap(mpvMemory, uiSize);

        }
        else
        {
            free(pvMemory);

            gcLogger.Error2(__METHOD__, " Could not resize file.", NULL);
            return SSharedMemoryMap();
        }
    }
    else
    {
        return SSharedMemoryMap(mpvMemory, uiSize);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryMap CResizableSharedMemory::IncreaseSize(size_t uiMore)
{
    size_t  uiSize;

    if (mpsDescriptor)
    {
        uiSize = (size_t)mpsDescriptor->uiSize + uiMore;
        uiSize = (size_t)(uiSize + uiSize / 2);
    }
    else
    {
        uiSize = 16000 + uiMore;
    }

    return Resize(uiSize);
}


