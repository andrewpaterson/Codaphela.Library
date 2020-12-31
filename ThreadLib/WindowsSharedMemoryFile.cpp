#include "BaseLib/Logger.h"
#include "ResizableSharedMemory.h"
#include "WindowsError.h"
#include "WindowsSharedMemoryFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult::SSharedMemoryResult(void)
{
    this->uiSize = 0;
    this->eResult = SMR_NotSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult::SSharedMemoryResult(size_t uiSize)
{
    this->uiSize = uiSize;
    this->eResult = SMR_Success;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult::SSharedMemoryResult(ESharedMemoryResult eRsult)
{
    this->uiSize = 0;
    this->eResult = eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SSharedMemoryResult::IsFailed(void)
{
    return eResult != SMR_Success;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SSharedMemoryResult::IsSuccess(void)
{
    return eResult == SMR_Success;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SSharedMemoryResult::IsInvalid(void)
{
    return eResult == SMR_Invalid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t SSharedMemoryResult::GetSize(void)
{
    return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryFile::Init(void)
{
    memset(this, 0, sizeof(CWindowsSharedMemoryFile));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryFile::Init(char* szName)
{
	mhMapFile = NULL;
    StrCpySafe(mszName, szName, 64);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryFile::Reinit(void)
{
    Kill();
    Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryFile::Reinit(char* szName)
{
    Kill();
    Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWindowsSharedMemoryFile::Kill(void)
{
	if (mhMapFile != NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Kill windows shared memory file [", mszName, "].  It's handle has not been closed.", NULL);
	}
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWindowsSharedMemoryFile::IsNamed(void)
{
    if (StrEmpty(mszName))
    {
        return FALSE;
    }
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CWindowsSharedMemoryFile::GetName(void)
{
    return mszName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CWindowsSharedMemoryFile::Create(size_t uiSize)
{
    size_t          iAdjustedSize;
    SResizableSharedMemory*  psDescriptor;

    iAdjustedSize = uiSize + sizeof(SResizableSharedMemory);

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mszName);
    if (mhMapFile)
    {
        CloseHandle(mhMapFile);
        mhMapFile = NULL;
        gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  File already exists.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,                       // maximum object size (high-order DWORD)
        iAdjustedSize,           // maximum object size (low-order DWORD)
        mszName);

    if (mhMapFile == NULL)
    {
        gcLogger.Warning2(__METHOD__, " Could not create file mapping object [", mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotCreate);
    }

    psDescriptor = Map(0);
    if (psDescriptor == NULL)
    {
        CloseHandle(mhMapFile);
        gcLogger.Warning2(__METHOD__, " Could not map view of file  [", mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotMap);
    }

    memset(psDescriptor, 0, sizeof(SResizableSharedMemory));
    psDescriptor->uiMagic = SHARED_MEMORY_MAGIC;
    psDescriptor->uiSize = uiSize;
    psDescriptor->iInvalid = SHARED_MEMORY_VALID;
    psDescriptor->iMapCount = 0;
    strcpy(psDescriptor->szName, "Not Set");

    UnmapViewOfFile(psDescriptor);

    return SSharedMemoryResult(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CWindowsSharedMemoryFile::Open(void)
{
    SResizableSharedMemory*  psDescriptor;
    size_t          uiSize;

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not open mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mszName);

    if (mhMapFile == NULL)
    {
        return SSharedMemoryResult(SMR_CannotOpen);
    }

    psDescriptor = Map(0);
  
    if (psDescriptor == NULL)
    {
        CloseHandle(mhMapFile);
        gcLogger.Warning2(__METHOD__, " Could not map view of file  [", mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotMap);
    }

    if (psDescriptor->iInvalid == SHARED_MEMORY_INVALID)
    {
        UnmapViewOfFile(psDescriptor);
        CloseHandle(mhMapFile);
        return SSharedMemoryResult(SMR_Invalid);
    }

    uiSize = (size_t)psDescriptor->uiSize;
    UnmapViewOfFile(psDescriptor);

    return SSharedMemoryResult(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryFile::Close(void)
{
    if (mhMapFile)
    {
        CloseHandle(mhMapFile);
        mhMapFile = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SResizableSharedMemory* CWindowsSharedMemoryFile::Map(size_t uiSize)
{
    SResizableSharedMemory*  psDescriptor;
    size_t          iAdjustedSize;

    if (mhMapFile != NULL)
    {
        iAdjustedSize = uiSize + sizeof(SResizableSharedMemory);

        psDescriptor = (SResizableSharedMemory*)MapViewOfFile(
            mhMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            iAdjustedSize);

        return psDescriptor;
    }
    else
    {
        return NULL;
    }
}

