#include "SharedMemory.h"
#include "WindowsSharedMemoryCoordinator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SWindowsSharedMemoryFileName::Init(int iIndex, char* szName)
{
	StrCpySafe(mszName, szName, 64);
	miIndex = iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWindowsSharedMemoryCoordinator::Init(char* szCoordinatorMemoryName)
{
	mcMemory.Init(szCoordinatorMemoryName);
	SSharedMemoryResult sResult = mcMemory.Open();
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SWindowsSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients++;
		return TRUE;
	}
	
	sResult = mcMemory.Create(8192);
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SWindowsSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients = 1;
		StrCpySafe(mpsDescriptor->szName, szCoordinatorMemoryName, 64);
		mpsDescriptor->iNumSharedMemoryFiles = 0;
		mpsDescriptor->iValidSharedMemoryIndex = -1;
		memset(&mpsDescriptor->macMemoryFiles[0], 0, sizeof(CWindowsSharedMemoryFile));
		return TRUE;
	}
	
	mpsDescriptor = NULL;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowsSharedMemoryCoordinator::Kill(void)
{
	if (mpsDescriptor)
	{
		mpsDescriptor->miClients--;
		UnmapViewOfFile(mpsDescriptor);
		mcMemory.Close();
		mpsDescriptor = NULL;
	}

	mcMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SWindowsSharedMemoryFileName* CWindowsSharedMemoryCoordinator::Add(char* szName)
{
	CChars							sz;
	SWindowsSharedMemoryFileName*	pcMemoryFile;

	if (mpsDescriptor)
	{
		if (mpsDescriptor->iValidSharedMemoryIndex != -1)
		{
			return FALSE;
		}

		sz.Init(szName);
		sz.Append(":");
		sz.Append(mpsDescriptor->iNumSharedMemoryFiles);
		if (sz.Length() >= 64)
		{
			sz.Kill();
			return FALSE;
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
SWindowsSharedMemoryFileName* CWindowsSharedMemoryCoordinator::GetCurrent(void)
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
void CWindowsSharedMemoryCoordinator::Invalidate(void)
{
	mpsDescriptor->iValidSharedMemoryIndex = -1;
}

