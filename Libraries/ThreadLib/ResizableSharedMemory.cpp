#include "BaseLib/Logger.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/Timer.h"
#include "BaseLib/NativeThread.h"
#include "InterProcessName.h"
#include "ResizableSharedMemory.h"


//The longest name the files are named after.  Each file is that name, a ':' and
//an index of up to three digits (the coordinator holds about a hundred).
#define RESIZABLE_SHARED_MEMORY_NAME_LENGTH	(SHARED_MEMORY_NAME_LENGTH - 4)

//How long a client that is leaving waits for another to take the memory only it still holds.
#define RESIZABLE_SHARED_MEMORY_HAND_OVER_MILLISECONDS	5000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CResizableSharedMemory::Init(char* szMemoryName, char* szDebugIdentifier)
{
    bool    bResult;

    bResult = InitCoordinator(szMemoryName);
    mcHoldingMappedFile.Init();
    mpsHoldingDescriptor = NULL;

    ShortenInterProcessName(&mszSharedName, szMemoryName, RESIZABLE_SHARED_MEMORY_NAME_LENGTH);

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
bool CResizableSharedMemory::InitCoordinator(char* szMemoryName)
{
    CChars  sz;
    bool    bResult;

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
bool CResizableSharedMemory::ReinitCoordinator(void)
{
    mpcCoordinatorMappedFile = NULL;
    mcMappedFile.Close();
    mcMappedFile.Reinit();
    mpsDescriptor = NULL;
    mpvMemory = NULL;

    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::Kill(void)
{
    Kill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// bHandOver is true when what the memory holds still matters to the other
// clients (a queue with elements in it).  It is not the default: a client
// can't tell another that is idle, or waiting on this one in the same
// thread, from one that will soon map the memory.
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::Kill(bool bHandOver)
{
    if (mpsHoldingDescriptor)
    {
        if (bHandOver)
        {
            HandOver();
        }
        mcHoldingMappedFile.Unmap(mpsHoldingDescriptor);
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
bool CResizableSharedMemory::Create(size_t uiSize)
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
        StrCpySafe(mpsDescriptor->szName, mszSharedName.Text(), SHARED_MEMORY_NAME_SIZE);
        return true;
    }
    else
    {
        return true;
    }

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CResizableSharedMemory::Connect(void)
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
    bool    bResult;

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
bool CResizableSharedMemory::Map(size_t uiSize)
{
    mpsDescriptor = mcMappedFile.Map(uiSize);
    if (mpsDescriptor == NULL)
    {
        gcLogger.Error2(__METHOD__, " Could not map view of file [", mpcCoordinatorMappedFile->mszName, "].  Failed with [", mcMappedFile.GetLastErrorString(), "].", NULL);
        return ReinitCoordinator();
    }

    mpsDescriptor->iMapCount++;
    mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CResizableSharedMemory::Remap(size_t uiSize)
{
    uint64                      uiOldSize;

    if (mpsDescriptor)
    {
        uiOldSize = mpsDescriptor->uiSize;
        mpsDescriptor->iMapCount--;
        mcMappedFile.Unmap(mpsDescriptor);
     
        mpsDescriptor = mcMappedFile.Map(uiSize);
        if (mpsDescriptor == NULL)
        {
            mpvMemory = NULL;
            mpsDescriptor = mcMappedFile.Map((size_t)uiOldSize);
            if (mpsDescriptor == NULL)
            {
                gcLogger.Error2(__METHOD__, " Could not re-map view of file [", mpcCoordinatorMappedFile->mszName, "] back to original size.  Failed with [", mcMappedFile.GetLastErrorString(), "].", NULL);
                return ReinitCoordinator();
            }
            mpsDescriptor->iMapCount++;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
            return false;
        }
        else
        {
            mpsDescriptor->iMapCount++;
            mpsDescriptor->uiSize = uiSize;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
            return true;
        }
    }
    else
    {
        return false;
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
            mcHoldingMappedFile.Unmap(mpsHoldingDescriptor);
            mcHoldingMappedFile.Close();
            mpsHoldingDescriptor = NULL;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The memory can be in a file only this client still has open: a Resize
// makes the new file and the other clients only open it when they next
// Touch.  The last handle closing removes the file, and the memory with it,
// so while other clients are attached this waits for one of them to map it
// (or for it to move to another file) before Kill lets go.
//
//////////////////////////////////////////////////////////////////////////
void CResizableSharedMemory::HandOver(void)
{
    volatile SSharedMemoryDescriptor*   psDescriptor;
    CTimer                              cTimer;

    psDescriptor = mpsHoldingDescriptor;
    if ((psDescriptor->iInvalid != SHARED_MEMORY_VALID) || (psDescriptor->iMapCount != 0) || (mcCoordinator.NumClients() <= 1))
    {
        return;
    }

    cTimer.Init();
    while ((psDescriptor->iInvalid == SHARED_MEMORY_VALID) && (psDescriptor->iMapCount == 0) && (mcCoordinator.NumClients() > 1))
    {
        cTimer.Update();
        if (cTimer.GetTotalTimeInMillieconds() >= RESIZABLE_SHARED_MEMORY_HAND_OVER_MILLISECONDS)
        {
            gcLogger.Error2(__METHOD__, " Could not hand over shared memory [", mcHoldingMappedFile.GetName(), "].  No other client took it, so it is lost.", NULL);
            break;
        }
        ThreadYield();
    }
    cTimer.Kill();
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

        mcMappedFile.Unmap(mpsDescriptor);
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
void* CResizableSharedMemory::GetValidMemory(void)
{
    if (mpsDescriptor == NULL)
    {
        return NULL;
    }

    //Another process sets iInvalid when it moves the memory to a new file.
    if (*(volatile int*)&mpsDescriptor->iInvalid != SHARED_MEMORY_VALID)
    {
        return NULL;
    }
    return mpvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryMap CResizableSharedMemory::Resize(size_t uiSize)
{
    bool                        bResult;
    uint64                      uiOldSize;
    void*                       pvMemory;
    CStackMemory<>              cTemp;

    bResult = Remap(uiSize);
    if (!bResult)
    { 
        mcCoordinator.Invalidate();
        mpsDescriptor->iInvalid = SHARED_MEMORY_INVALID;
        uiOldSize = mpsDescriptor->uiSize;
        pvMemory = cTemp.Init((size_t)uiOldSize);
        memcpy(pvMemory, mpvMemory, (size_t)uiOldSize);

        Close();
        bResult = Create(uiSize);
        if (bResult)
        {
            memcpy(mpvMemory, pvMemory, (size_t)uiOldSize);
            cTemp.Kill();
         
            return SSharedMemoryMap(mpvMemory, uiSize);

        }
        else
        {
            cTemp.Kill();

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


