#include "BaseLib/Logger.h"
#include "SharedMemory.h"
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
void CWindowsSharedMemoryFile::Init(char* szName, int iIndex)
{
	mhMapFile = NULL;
	miIndex = iIndex;
	mszName.Init(szName);
	mszName.Append(":");
	mszName.Append(iIndex + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWindowsSharedMemoryFile::Kill(void)
{
	if (mhMapFile != NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Kill windows shared memory file.  It's handle has not been closed.", NULL);
	}
	mszName.Kill();
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CWindowsSharedMemoryFile::Create(size_t uiSize)
{
    size_t          iAdjustedSize;
    SSharedMemory*  psDescriptor;

    iAdjustedSize = uiSize + sizeof(SSharedMemory);

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not create mapping.  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mszName.Text());
    if (mhMapFile)
    {
        CloseHandle(mhMapFile);
        mhMapFile = NULL;
        gcLogger.Error2(__METHOD__, " Could not create mapping.  File alread exists.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,                       // maximum object size (high-order DWORD)
        iAdjustedSize,           // maximum object size (low-order DWORD)
        mszName.Text());

    if (mhMapFile == NULL)
    {
        gcLogger.Warning2(__METHOD__, " Could not create file mapping object [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotCreate);
    }

    psDescriptor = Map(0);
    if (psDescriptor == NULL)
    {
        CloseHandle(mhMapFile);
        gcLogger.Warning2(__METHOD__, " Could not map view of file [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotMap);
    }

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
    SSharedMemory*  psDescriptor;
    size_t          uiSize;

    if (mhMapFile)
    {
        gcLogger.Error2(__METHOD__, " Could not open mapping.  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mszName.Text());

    if (mhMapFile == NULL)
    {
        return SSharedMemoryResult(SMR_CannotOpen);
        //return gcLogger.Error2(__METHOD__, " Could not open file mapping object [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
    }

    psDescriptor = Map(0);
  
    if (psDescriptor == NULL)
    {
        CloseHandle(mhMapFile);
        gcLogger.Warning2(__METHOD__, " Could not map view of file [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
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
SSharedMemory* CWindowsSharedMemoryFile::Map(size_t uiSize)
{
    SSharedMemory*  psDescriptor;
    size_t          iAdjustedSize;

    if (mhMapFile != NULL)
    {
        iAdjustedSize = uiSize + sizeof(SSharedMemory);

        psDescriptor = (SSharedMemory*)MapViewOfFile(
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

