#include "BaseLib/Logger.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/StringHelper.h"
#include "SharedMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Init(char* szName)
{
    mszName.Init(szName);
    miConnectionId = 0;
    mpvMemory = NULL;
    mpsDescriptor = NULL;
    mhMapFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Kill(void)
{
    mszName.Kill();
    miConnectionId = 0;
    mpvMemory = NULL;
    mpsDescriptor = NULL;
    mhMapFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Create(size_t uiSize)
{
    size_t iAdjustedSize;

    iAdjustedSize = uiSize + sizeof(SSharedMemory);

    mhMapFile = CreateFileMapping(
                    INVALID_HANDLE_VALUE,    // use paging file
                    NULL,                    // default security
                    PAGE_READWRITE,          // read/write access
                    0,                       // maximum object size (high-order DWORD)
                    iAdjustedSize,           // maximum object size (low-order DWORD)
                    mszName.Text());                 // name of mapping object

    if (mhMapFile == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not create file mapping object [", GetLastError(),"].", NULL);
    }

    mpsDescriptor = (SSharedMemory*)MapViewOfFile(
                    mhMapFile,               // handle to map object
                    FILE_MAP_ALL_ACCESS,    // read/write permission
                    0,
                    0,
                    iAdjustedSize);

   if (mpsDescriptor == NULL)
   {
       CloseHandle(mhMapFile);
       return gcLogger.Error2(__METHOD__, " Could not map view of file [", GetLastError(), "].", NULL);
   }

   mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));

   miConnectionId = 1;

   mpsDescriptor->uiMagic = SHARED_MEMORY_MAGIC;
   mpsDescriptor->uiSize = uiSize;
   mpsDescriptor->iConnectionId = miConnectionId;
   StrCpySafe(mpsDescriptor->szName, mszName.Text(), 128);

   return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Connect(void)
{
    return Touch(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Touch(size_t uiSize)
{
    int     iAdjustedSize;
    int     iConnectionId;

    iAdjustedSize = uiSize + sizeof(SSharedMemory);

    mhMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        mszName.Text());       // name of mapping object

    if (mhMapFile == NULL)
    {
        return gcLogger.Error2(__METHOD__, " Could not open file mapping object [", GetLastError(), "].", NULL);
    }

    mpsDescriptor = (SSharedMemory*)MapViewOfFile(
        mhMapFile,              // handle to map object
        FILE_MAP_ALL_ACCESS,    // read/write permission
        0,
        0,
        iAdjustedSize);

    if (mpsDescriptor == NULL)
    {
        CloseHandle(mhMapFile);
        return gcLogger.Error2(__METHOD__, " Could not map view of file [", GetLastError(), "].", NULL);
    }

    if (mpsDescriptor->uiMagic != SHARED_MEMORY_MAGIC)
    {
        Close();
        return gcLogger.Error2(__METHOD__, " Magic mismatch connecting to view of file [", LongLongToString(mpsDescriptor->uiMagic, 16), "].", NULL);
    }

    //Test that the names are the same.

    if (miConnectionId != mpsDescriptor->iConnectionId)
    {
        iConnectionId = mpsDescriptor->iConnectionId;
        uiSize = (size_t)mpsDescriptor->uiSize;
        Close();
        miConnectionId = iConnectionId;
        return Touch(uiSize);
    }
    else
    {
        mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemory));
        return TRUE;
    }
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

    CloseHandle(mhMapFile);
    mhMapFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::GetMemory(void)
{
    return mpvMemory;
}

