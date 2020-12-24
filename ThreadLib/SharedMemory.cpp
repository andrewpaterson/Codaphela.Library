#include "BaseLib/Logger.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/StringHelper.h"
#include "SharedMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Create(char* szName, size_t iSize)
{
    size_t iAdjustedSize;

    mpvMemory = NULL;
    mpsDescriptor = NULL;
    iAdjustedSize = iSize + sizeof(SSharedMemory);

    hMapFile = CreateFileMapping(
                    INVALID_HANDLE_VALUE,    // use paging file
                    NULL,                    // default security
                    PAGE_READWRITE,          // read/write access
                    0,                       // maximum object size (high-order DWORD)
                    iAdjustedSize,                   // maximum object size (low-order DWORD)
                    szName);                 // name of mapping object

    if (hMapFile == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not create file mapping object [", GetLastError(),"].", NULL);
    }

    mpsDescriptor = (SSharedMemory*)MapViewOfFile(
                    hMapFile,               // handle to map object
                    FILE_MAP_ALL_ACCESS,    // read/write permission
                    0,
                    0,
                    iAdjustedSize);

   if (mpsDescriptor == NULL)
   {
       CloseHandle(hMapFile);
       return gcLogger.Error2(__METHOD__, " Could not map view of file [", GetLastError(), "].", NULL);
   }

   mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));

   mpsDescriptor->uiMagic = SHARED_MEMORY_MAGIC;
   mpsDescriptor->uiSize = iSize;
   StrCpySafe(mpsDescriptor->szName, szName, 128);

   return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Connect(char* szName, size_t iSize)
{
    int iAdjustedSize;

    mpvMemory = NULL;
    mpsDescriptor = NULL;
    iAdjustedSize = iSize + sizeof(SSharedMemory);

    hMapFile = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,   // read/write access
                    FALSE,                 // do not inherit the name
                    szName);               // name of mapping object

    if (hMapFile == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not open file mapping object [", GetLastError(), "].", NULL);
    }

    mpsDescriptor = (SSharedMemory*)MapViewOfFile(
                    hMapFile,               // handle to map object
                    FILE_MAP_ALL_ACCESS,    // read/write permission
                    0,
                    0,
                    iAdjustedSize);

    if (mpsDescriptor == NULL)
    {
        CloseHandle(hMapFile);
        return gcLogger.Error2(__METHOD__, " Could not map view of file [", GetLastError(), "].", NULL);
    }

    if (mpsDescriptor->uiMagic != SHARED_MEMORY_MAGIC)
    {
        Close();
        return gcLogger.Error2(__METHOD__, " Magic mismatch connecting to view of file [", LongLongToString(mpsDescriptor->uiMagic, 16), "].", NULL);
    }

    mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Close(void)
{
    UnmapViewOfFile(mpsDescriptor);
    mpsDescriptor = NULL;
    mpvMemory = NULL;

    CloseHandle(hMapFile);
    hMapFile = INVALID_HANDLE_VALUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::GetMemory(void)
{
    return mpvMemory;
}

