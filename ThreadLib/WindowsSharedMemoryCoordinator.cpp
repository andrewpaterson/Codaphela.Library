#include "ResizableSharedMemory.h"
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
bool CWindowsSharedMemoryCoordinator::Init(char* szCoordinatorMemoryName)
{
	mcMemory.Init(szCoordinatorMemoryName);
	SSharedMemoryResult sResult = mcMemory.Open();
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SResizableWindowsSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients++;
		return true;
	}
	
	sResult = mcMemory.Create(8192);
	if (sResult.IsSuccess())
	{
		mpsDescriptor = (SResizableWindowsSharedMemoryFile*)mcMemory.Map(sResult.GetSize());
		mpsDescriptor->miClients = 1;
		StrCpySafe(mpsDescriptor->szName, szCoordinatorMemoryName, 64);
		mpsDescriptor->iNumSharedMemoryFiles = 0;
		mpsDescriptor->iValidSharedMemoryIndex = -1;
		memset(&mpsDescriptor->macMemoryFiles[0], 0, sizeof(CWindowsSharedMemoryFile));
		return true;
	}
	
	mpsDescriptor = NULL;
	return false;
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
			return false;
		}

		sz.Init(szName);
		sz.Append(":");
		sz.Append(mpsDescriptor->iNumSharedMemoryFiles);
		if (sz.Length() >= 64)
		{
			sz.Kill();
			return false;
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

