#ifdef _WIN32
#include "BaseLib/Logger.h"
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/WindowsError.h"
#include "InterProcessName.h"
#include "SharedMemoryFile.h"


static_assert(sizeof(HANDLE) <= SHARED_MEMORY_FILE_HANDLE_SIZE, "SHARED_MEMORY_FILE_HANDLE_SIZE is too small for a HANDLE.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static HANDLE& MapFileHandle(uint8* paMapFile)
{
    return *(HANDLE*)paMapFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(void)
{
    memset(this, 0, sizeof(CSharedMemoryFile));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Init(char* szName)
{
	CChars	sz;

	MapFileHandle(maMapFile) = NULL;
	ShortenInterProcessName(&sz, szName, SHARED_MEMORY_NAME_LENGTH);
	StrCpySafe(mszName, sz.Text(), SHARED_MEMORY_NAME_SIZE);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryFile::Kill(void)
{
	if (MapFileHandle(maMapFile) != NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Kill windows shared memory file [", mszName, "].  It's handle has not been closed.", NULL);
	}
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryResult CSharedMemoryFile::Create(size_t uiSize)
{
    size_t          iAdjustedSize;
    SSharedMemoryDescriptor*  psDescriptor;

    iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

    if (MapFileHandle(maMapFile))
    {
        gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    MapFileHandle(maMapFile) = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        false,
        mszName);
    if (MapFileHandle(maMapFile))
    {
        CloseHandle(MapFileHandle(maMapFile));
        MapFileHandle(maMapFile) = NULL;
        gcLogger.Error2(__METHOD__, " Could not create mapping [", mszName, "].  File already exists.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    MapFileHandle(maMapFile) = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,                       // maximum object size (high-order DWORD)
        iAdjustedSize,           // maximum object size (low-order DWORD)
        mszName);

    if (MapFileHandle(maMapFile) == NULL)
    {
        gcLogger.Warning2(__METHOD__, " Could not create file mapping object [", mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotCreate);
    }

    psDescriptor = Map(0);
    if (psDescriptor == NULL)
    {
        CloseHandle(MapFileHandle(maMapFile));
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
SSharedMemoryResult CSharedMemoryFile::Open(void)
{
    SSharedMemoryDescriptor*  psDescriptor;
    size_t          uiSize;

    if (MapFileHandle(maMapFile))
    {
        gcLogger.Error2(__METHOD__, " Could not open mapping [", mszName, "].  Map file handle must be NULL.", NULL);
        return SSharedMemoryResult(SMR_FileHandleAlreadySet);
    }

    MapFileHandle(maMapFile) = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        false,
        mszName);

    if (MapFileHandle(maMapFile) == NULL)
    {
        return SSharedMemoryResult(SMR_CannotOpen);
    }

    psDescriptor = Map(0);
  
    if (psDescriptor == NULL)
    {
        CloseHandle(MapFileHandle(maMapFile));
        gcLogger.Warning2(__METHOD__, " Could not map view of file  [", mszName, "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
        return SSharedMemoryResult(SMR_CannotMap);
    }

    if (psDescriptor->iInvalid == SHARED_MEMORY_INVALID)
    {
        UnmapViewOfFile(psDescriptor);
        CloseHandle(MapFileHandle(maMapFile));
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
void CSharedMemoryFile::Close(void)
{
    if (MapFileHandle(maMapFile))
    {
        CloseHandle(MapFileHandle(maMapFile));
        MapFileHandle(maMapFile) = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryDescriptor* CSharedMemoryFile::Map(size_t uiSize)
{
    SSharedMemoryDescriptor*  psDescriptor;
    size_t          iAdjustedSize;

    if (MapFileHandle(maMapFile) != NULL)
    {
        iAdjustedSize = uiSize + sizeof(SSharedMemoryDescriptor);

        psDescriptor = (SSharedMemoryDescriptor*)MapViewOfFile(
            MapFileHandle(maMapFile),
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryFile::Unmap(SSharedMemoryDescriptor* psDescriptor)
{
    UnmapViewOfFile(psDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSharedMemoryFile::GetLastErrorString(void)
{
    return WindowsErrorCodeToString(GetLastError());
}


#endif // _WIN32

