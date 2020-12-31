#include "BaseLib/Logger.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/StringHelper.h"
#include "WindowsError.h"
#include "SharedMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Init(char* szMemoryName, char* szDebugIdentifier)
{
    InitCoordinator(szMemoryName);

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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::InitCoordinator(char* szMemoryName)
{
    CChars                      sz;

    sz.Init(szMemoryName);
    sz.Append(":C");
    mcCoordinator.Init(sz.Text());
    sz.Kill();

    //mpcCoordinatorMappedFile, mcMappedFile, mpvMemory and mpsDescriptor all move together
    mpcCoordinatorMappedFile = NULL;

    mcMappedFile.Init();

    mpvMemory = NULL;
    mpsDescriptor = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::ReinitCoordinator(void)
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
void CSharedMemory::Kill(void)
{
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
BOOL CSharedMemory::Create(size_t uiSize)
{
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
BOOL CSharedMemory::Connect(void)
{
    if (mcMappedFile.IsNamed())
    {
        return gcLogger.Error2(__METHOD__, " Could not connect to map file.  Already connected [", mcMappedFile.GetName(), "].", NULL);
    }

    mpcCoordinatorMappedFile = mcCoordinator.GetCurrent();
    if (mpcCoordinatorMappedFile)
    {
        mcMappedFile.Reinit(mpcCoordinatorMappedFile->mszName);

        SSharedMemoryResult sResult = mcMappedFile.Open();
        if (sResult.IsFailed())
        {
            ReinitCoordinator();
            return gcLogger.Error2(__METHOD__, " Could not connect to map file.  No map file exists.", NULL);
        }

        return Map(sResult.GetSize());
    }
    else
    {
        ReinitCoordinator();
        return gcLogger.Error2(__METHOD__, " Could not connect to map file.  No map file exists.", NULL);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::Touch(void)
{
    BOOL                        bResult;

    if (mpsDescriptor)
    {
        if (mpsDescriptor->iInvalid)
        {
            CChars sz;
            sz.Init("Invalid ");
            sz.Append(mszDebugIdentifier);
            sz.Append(": ");
            sz.Append(mpsDescriptor->uiSize);
            sz.Append(" [");
            sz.Append(mpsDescriptor->iMapCount);
            sz.Append("]");
            sz.Dump();
            sz.Kill();

            Close();
            bResult = Connect();
            
            if (bResult)
            {
                sz.Init(" -> Connect: ");
                sz.Append(mpsDescriptor->uiSize);
                sz.Append(" [");
                sz.Append(mpsDescriptor->iMapCount);
                sz.Append("]");
                sz.AppendNewLine();
                sz.Dump();
                sz.Kill();
            }
            else
            {
                sz.Init(" -> Connect Failed");
                sz.AppendNewLine();
                sz.Dump();
                sz.Kill();
            }

            if (bResult)
            {
                return mpvMemory;
            }
            return NULL;
        }
        else 
        {
            //Test the name and the map count.
            return mpvMemory;
        }
    }
    else
    {
        Close();
        if (Connect())
        {
            return mpvMemory;
        }
        return NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Map(size_t uiSize)
{
    mpsDescriptor = mcMappedFile.Map(uiSize);
    if (mpsDescriptor == NULL)
    {
        ReinitCoordinator();
        return gcLogger.Error2(__METHOD__, " Could not map view of file [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }

    mpsDescriptor->iMapCount++;
    mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Remap(size_t uiSize)
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
                ReinitCoordinator();
                return gcLogger.Error2(__METHOD__, " Could not re-map view of file back to original size.  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
            }
            mpsDescriptor->iMapCount++;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));
            return FALSE;
        }
        else
        {
            mpsDescriptor->iMapCount++;
            mpsDescriptor->uiSize = uiSize;
            mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));
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
int CSharedMemory::Close(void)
{
    int                         iStillMapped;

    if (mpsDescriptor)
    {
        mpsDescriptor->iMapCount--;
        iStillMapped = mpsDescriptor->iMapCount;

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
size_t CSharedMemory::GetSize(void)
{
    return (size_t)mpsDescriptor->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::Resize(size_t uiSize)
{
    BOOL                        bResult;
    uint64                      uiOldSize;
    int                         iStillMapped;
    void*                       pvMemory;
    CChars                      sz;

    bResult = Remap(uiSize);
    if (!bResult)
    { 
        mcCoordinator.Invalidate();
        mpsDescriptor->iInvalid = SHARED_MEMORY_INVALID;
        uiOldSize = mpsDescriptor->uiSize;
        pvMemory = malloc((size_t)uiOldSize);
        memcpy(pvMemory, mpvMemory, (size_t)uiOldSize);

        sz.Init("Resized ");
        sz.Append(mszDebugIdentifier);
        sz.Append(": ");
        sz.Append(uiSize);
        sz.Append(" [");
        sz.Append(mpsDescriptor->iMapCount);
        sz.Append("]");
        sz.Dump();
        sz.Kill();

        iStillMapped = Close();
        bResult = Create(uiSize);
        if (bResult)
        {
            memcpy(mpvMemory, pvMemory, (size_t)uiOldSize);
            free(pvMemory);
            
            sz.Init(" -> Close: ");
            sz.Append(0);
            sz.Append(" [");
            sz.Append(iStillMapped);
            sz.Append("]");
            sz.AppendNewLine();
            sz.Dump();
            sz.Kill();


            return mpvMemory;
        }
        else
        {
            free(pvMemory);

            sz.Init(" -> Resized Failed");
            sz.AppendNewLine();
            sz.Dump();
            sz.Kill();

            gcLogger.Error2(__METHOD__, " Could not resize file.", NULL);
            return NULL;
        }
    }
    else
    {
        return mpvMemory;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::IncreaseSize(void)
{
    size_t  uiSize;

    if (mpsDescriptor)
    {
        uiSize = (size_t)(mpsDescriptor->uiSize + mpsDescriptor->uiSize / 2);
    }
    else
    {
        uiSize = 16000;
    }

    return Resize(uiSize);
}

