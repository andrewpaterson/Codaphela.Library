#include "BaseLib/Logger.h"
#include "BaseLib/WindowsError.h"
#include "WindowsSharedMemoryFile.h"


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
bool CWindowsSharedMemoryFile::Kill(void)
{
	if (mhMapFile != NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Kill windows shared memory file [", mszName, "].  It's handle has not been closed.", NULL);
	}
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowsSharedMemoryFile::IsNamed(void)
{
    if (StrEmpty(mszName))
    {
        return false;
    }
    return true;
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
    SSharedMemoryDescriptor*  psDescriptor;

    iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        false,
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

    memset(psDescriptor, 0, sizeof(SSharedMemoryDescriptor));
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
    SSharedMemoryDescriptor*  psDescriptor;
    size_t          uiSize;

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not open mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        false,
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
SSharedMemoryDescriptor* CWindowsSharedMemoryFile::Map(size_t uiSize)
{
    SSharedMemoryDescriptor*  psDescriptor;
    size_t          iAdjustedSize;

    if (mhMapFile != NULL)
    {
        iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

        psDescriptor = (SSharedMemoryDescriptor*)MapViewOfFile(
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

