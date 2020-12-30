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
void CSharedMemory::Init(char* szName)
{
    mcMappedFiles[0].Init(szName, 0);
    mcMappedFiles[1].Init(szName, 1);
    mszSharedName.Init(szName);
    mpvMemory = NULL;
    mpsDescriptor = NULL;
    miSharedMemory = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Kill(void)
{
    mszSharedName.Kill();
    mcMappedFiles[0].Kill();
    mcMappedFiles[1].Kill();
    mpvMemory = NULL;
    mpsDescriptor = NULL;
    miSharedMemory = -1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Create(size_t uiSize)
{
    return Create(0, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Create(int iSharedMemory, size_t uiSize)
{
    miSharedMemory = iSharedMemory;

    SSharedMemoryResult sResult = mcMappedFiles[iSharedMemory].Create(uiSize);
    if (sResult.IsFailed())
    {
        return FALSE;
    }

    mpsDescriptor = mcMappedFiles[iSharedMemory].Map(uiSize);

   if (mpsDescriptor == NULL)
   {
       mcMappedFiles[iSharedMemory].Close();
       return gcLogger.Error2(__METHOD__, " Could not map view of file [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
   }

   mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));

   mpsDescriptor->iMapCount = 1;
   StrCpySafe(mpsDescriptor->szName, mszSharedName.Text(), 128);

   return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Connect(void)
{
    BOOL    bResult;

    if ((miSharedMemory == -1) || (miSharedMemory == 0))
    {
        bResult = Connect(0, 1);
        if (!bResult)
        {
            return Connect(1, 0);
        }
        return TRUE;
    }
    else
    {
        bResult = Connect(1, 0);
        if (!bResult)
        {
            return Connect(0, 1);
        }
        return TRUE;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Connect(int iCurrentMemory, int iNextMemory)
{
    miSharedMemory = iCurrentMemory;

    SSharedMemoryResult sResult = mcMappedFiles[iCurrentMemory].Open();

    if (sResult.IsSuccess())
    {
        return Map(iCurrentMemory, sResult.GetSize());
    }
    else if (sResult.IsInvalid())
    {
        miSharedMemory = iNextMemory;

        SSharedMemoryResult sResult = mcMappedFiles[iNextMemory].Open();

        if (sResult.IsSuccess())
        {
            return Map(iNextMemory, sResult.GetSize());
        }
        else if (sResult.IsInvalid())
        {
            return FALSE;
        }
        else
        {
            return FALSE;
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
BOOL CSharedMemory::Touch(void)
{
    int     iSharedMemory;
    BOOL    bResult;

    if (mpsDescriptor)
    {
        if (mpsDescriptor->iInvalid)
        {
            CChars sz;
            sz.Init("Invalid: ");
            sz.Append(mpsDescriptor->uiSize);
            sz.Append(" [");
            sz.Append(mpsDescriptor->iMapCount);
            sz.Append("]");
            sz.Dump();
            sz.Kill();

            iSharedMemory = miSharedMemory;
            Close();
            bResult = Connect(1 - iSharedMemory, iSharedMemory);

            sz.Init(" -> Connect: ");
            sz.Append(mpsDescriptor->uiSize);
            sz.Append(" [");
            sz.Append(mpsDescriptor->iMapCount);
            sz.Append("]");
            sz.AppendNewLine();
            sz.Dump();
            sz.Kill();

            return bResult;
        }
        else if (mpsDescriptor->szName[0] == '\0')
        {
            return gcLogger.Error2(__METHOD__, " Touched a descriptor with an invalid name.", NULL);
        }
        else
        {
            //Test the name and the map count.
            return TRUE;
        }
    }
    else
    {
        return Connect();
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Map(int iSharedMemory, size_t uiSize)
{
    mpsDescriptor = mcMappedFiles[iSharedMemory].Map(uiSize);
    if (mpsDescriptor == NULL)
    {
        mpvMemory = NULL;
        mcMappedFiles[0].Close();
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
BOOL CSharedMemory::Remap(int iSharedMemory, size_t uiSize)
{
    uint64 uiOldSize;
        
    if (mpsDescriptor)
    {
        uiOldSize = mpsDescriptor->uiSize;
        mpsDescriptor->iMapCount--;
        UnmapViewOfFile(mpsDescriptor);
     
        mpsDescriptor = mcMappedFiles[iSharedMemory].Map(uiSize);
        if (mpsDescriptor == NULL)
        {
            mpvMemory = NULL;
            mpsDescriptor = mcMappedFiles[iSharedMemory].Map((size_t)uiOldSize);
            if (mpsDescriptor == NULL)
            {
                mpvMemory = NULL;
                mcMappedFiles[0].Close();
                return gcLogger.Error2(__METHOD__, " Could not re-map view of file [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
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
    int iStillMapped;

    iStillMapped = Close(mpsDescriptor, miSharedMemory);
    mpsDescriptor = NULL;
    mpvMemory = NULL;
    miSharedMemory = -1;

    return iStillMapped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSharedMemory::Close(SSharedMemory* psDescriptor, int iSharedMemory)
{
    int iStillMapped;

    if (psDescriptor)
    {
        psDescriptor->iMapCount--;
        iStillMapped = psDescriptor->iMapCount;
        if (psDescriptor->iMapCount == 0)
        {
            memset(psDescriptor, 0, sizeof(SSharedMemory));
            psDescriptor->iInvalid = SHARED_MEMORY_VALID;
            psDescriptor->iMapCount = -1;
        }
        UnmapViewOfFile(psDescriptor);
    }
    else
    {
        iStillMapped = 0;
    }

    mcMappedFiles[iSharedMemory].Close();
    return iStillMapped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::GetMemory(void)
{
    return mpvMemory;
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
BOOL CSharedMemory::Resize(size_t uiSize)
{
    BOOL            bResult;
    SSharedMemory*  psOldDescriptor;
    int             iOldSharedMemory;
    void*           pvOldMemory;
    uint64          uiOldSize;

    bResult = Remap(miSharedMemory, uiSize);
    if (!bResult)
    { 
        psOldDescriptor = mpsDescriptor;
        iOldSharedMemory = miSharedMemory;
        pvOldMemory = mpvMemory;
        uiOldSize = mpsDescriptor->uiSize;
        bResult = Create(1 - miSharedMemory, uiSize);
        if (bResult)
        {
            CChars sz;
            sz.Init("Resized: ");
            sz.Append(uiSize);
            sz.Append(" [");
            sz.Append(mpsDescriptor->iMapCount);
            sz.Append("]");
            sz.AppendNewLine();
            sz.Dump();
            sz.Kill();

            memcpy(mpvMemory, pvOldMemory, (size_t)uiOldSize);
            psOldDescriptor->iInvalid = SHARED_MEMORY_INVALID;
            Close(psOldDescriptor, iOldSharedMemory);
            return TRUE;
        }
        else
        {
            return gcLogger.Error2(__METHOD__, " Could not resize file.", NULL);
        }
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
size_t CSharedMemory::IncreaseSize(void)
{
    size_t  uiSize;
    BOOL    bResult;

    if (mpsDescriptor)
    {
        uiSize = (size_t)(mpsDescriptor->uiSize + mpsDescriptor->uiSize / 2);
    }
    else
    {
        uiSize = 16000;
    }

    bResult = Resize(uiSize);
    if (bResult)
    {
        return uiSize;
    }
    else
    {
        return 0;
    }
}

