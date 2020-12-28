#include "SharedMemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(BOOL bMaster, char* szName, size_t uiByteSize)
{
	BOOL		bResult;
	CChars		szMemoryName;
	CChars		szMutexName;

	mbMaster = bMaster;

	szMemoryName.Init("Local\\SharedMemory\\");
	szMemoryName.Append(szName);

	szMutexName.Init("Local\\ProcessMutex\\");
	szMutexName.Append(szName);

	mcMutex.Init(szMutexName.Text());

	szMutexName.Kill();


	if (mbMaster)
	{
		bResult = mcMemory.Create(uiByteSize);
		if (bResult)
		{
			bResult = mcMutex.Create();
			if (bResult)
			{
				mcQueue.Init(mcMemory.GetMemory(), uiByteSize);
			}
			else
			{
				mcMemory.Close();
			}
		}
	}
	else
	{

	}
	szMemoryName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryQueue::Kill(void)
{
	mcMutex.Kill();
}


