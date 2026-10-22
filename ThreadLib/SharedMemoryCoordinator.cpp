#include "ResizableSharedMemory.h"
#include "SharedMemoryCoordinator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSharedMemoryFileName::Init(int iIndex, char* szName)
{
	StrCpySafe(mszName, szName, SHARED_MEMORY_NAME_SIZE);
	miIndex = iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryCoordinator::Init(char* szCoordinatorMemoryName)
{
	mcMemory.Init(szCoordinatorMemoryName);
	SSharedMemoryResult sResult = mcMemory.Open();
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SResizableSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients.Increment();
		return true;
	}
	
	sResult = mcMemory.Create(8192);
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SResizableSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients.Set(1);
		StrCpySafe(mpsDescriptor->szName, szCoordinatorMemoryName, SHARED_MEMORY_NAME_SIZE);
		mpsDescriptor->iNumSharedMemoryFiles = 0;
		mpsDescriptor->iValidSharedMemoryIndex = -1;
		memset(&mpsDescriptor->macMemoryFiles[0], 0, sizeof(SSharedMemoryFileName));
		return true;
	}
	
	mpsDescriptor = NULL;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryCoordinator::Kill(void)
{
	if (mpsDescriptor)
	{
		mpsDescriptor->miClients.Decrement();
		mcMemory.Unmap(mpsDescriptor);
		mcMemory.Close();
		mpsDescriptor = NULL;
	}

	mcMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryFileName* CSharedMemoryCoordinator::Add(char* szName)
{
	CChars							sz;
	SSharedMemoryFileName*	pcMemoryFile;

	if (mpsDescriptor)
	{
		if (mpsDescriptor->iValidSharedMemoryIndex != -1)
		{
			return NULL;
		}

		sz.Init(szName);
		sz.Append(":");
		sz.Append(mpsDescriptor->iNumSharedMemoryFiles);
		if (sz.Length() > SHARED_MEMORY_NAME_LENGTH)
		{
			sz.Kill();
			return NULL;
		}

		pcMemoryFile = &mpsDescriptor->macMemoryFiles[mpsDescriptor->iNumSharedMemoryFiles];
		pcMemoryFile->Init(mpsDescriptor->iNumSharedMemoryFiles, sz.Text());
		sz.Kill();
		mpsDescriptor->iValidSharedMemoryIndex = mpsDescriptor->iNumSharedMemoryFiles;
		mpsDescriptor->iNumSharedMemoryFiles++;

		return pcMemoryFile;
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
SSharedMemoryFileName* CSharedMemoryCoordinator::GetCurrent(void)
{
	if ((mpsDescriptor->iValidSharedMemoryIndex != -1) && (mpsDescriptor->iValidSharedMemoryIndex < mpsDescriptor->iNumSharedMemoryFiles))
	{
		return &mpsDescriptor->macMemoryFiles[mpsDescriptor->iValidSharedMemoryIndex];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryCoordinator::Invalidate(void)
{
	mpsDescriptor->iValidSharedMemoryIndex = -1;
}

