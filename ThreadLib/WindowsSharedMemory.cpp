#include "BaseLib/Logger.h"
#include "WindowsError.h"
#include "SharedMemoryDescriptor.h"
#include "WindowsSharedMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Init(char* szName)
{
	mcMappedFile.Init(szName);
	mpsDescriptor = NULL;
	mpvMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemory::Kill(void)
{
	BOOL	bResult;

	bResult = mcMappedFile.Kill();
	mpsDescriptor = NULL;
	mpvMemory = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSharedMemory::GetName(void)
{
	return mcMappedFile.GetName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::Create(size_t uiSize)
{
	SSharedMemoryResult sResult = mcMappedFile.Create(uiSize);

	if (sResult.IsSuccess())
	{
		mpsDescriptor = mcMappedFile.Map(sResult.uiSize);
		mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
		return mpvMemory;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not create map file [", mcMappedFile.GetName(), "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedMemory::Open(void)
{
	SSharedMemoryResult sResult = mcMappedFile.Open();

	if (sResult.IsSuccess())
	{
		mpsDescriptor = mcMappedFile.Map(sResult.uiSize);
		mpvMemory = RemapSinglePointer(mpsDescriptor, sizeof(SSharedMemoryDescriptor));
		return mpvMemory;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not open map file [", mcMappedFile.GetName(), "].  Failed with [", WindowsErrorCodeToString(GetLastError()), "].", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemory::Close(void)
{
	if (mpsDescriptor)
	{
		UnmapViewOfFile(mpsDescriptor);
		mpsDescriptor = NULL;
	}
	mpvMemory = NULL;
	mcMappedFile.Close();
}

