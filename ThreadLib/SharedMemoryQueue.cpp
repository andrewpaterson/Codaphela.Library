#include "SharedMemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(BOOL bMaster, char* szName, size_t uiByteSize)
{
	CChars		szMemoryName;
	CChars		szMutexName;

	mbMaster = bMaster;

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();

	if (mbMaster)
	{
		return InitMaster(uiByteSize);
	}
	else
	{
		return InitClient();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::InitMaster(size_t uiByteSize)
{
	BOOL	bResult;
	void*	pvMemory;

	bResult = mcSharedMemory.Create(uiByteSize);
	if (!bResult)
	{
		mcSharedMemory.Close();
		return FALSE;
	}

	bResult = mcMutex.Create();
	if (!bResult)
	{
		mcMutex.Close();
		mcSharedMemory.Close();
		return FALSE;
	}

	pvMemory = mcSharedMemory.Touch();
	mcQueue.Init(pvMemory, mcSharedMemory.GetSize());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::InitClient(void)
{
	BOOL	bResult;
	void*	pvMemory;

	bResult = mcSharedMemory.Connect();
	if (!bResult)
	{
		mcSharedMemory.Close();
		return FALSE;
	}

	bResult = mcMutex.Connect();
	if (!bResult)
	{
		mcMutex.Close();
		mcSharedMemory.Close();
		return FALSE;
	}

	pvMemory = mcSharedMemory.Touch();
	mcQueue.Init(pvMemory, mcSharedMemory.GetSize());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryQueue::Kill(void)
{
	mcQueue.Kill();
	mcSharedMemory.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Push(void* pvData, size_t uiDataSize)
{
	void*	pvMemory;

	pvMemory = mcQueue.Push(uiDataSize);
	if (!pvMemory)
	{
		SSharedMemoryResize	sResize = mcSharedMemory.IncreaseSize(uiDataSize);
		if (!sResize.pvMemory)
		{
			return FALSE;
		}

		mcQueue.Remap(sResize.pvMemory, sResize.uiSize);
		pvMemory = mcQueue.Push(uiDataSize);
		if (!pvMemory)
		{
			return FALSE;
		}
	}

	memcpy(pvMemory, pvData, uiDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Pop(void* pvData, size_t* puiDataSize, size_t uiMaxDataSize)
{
	void*	pvMemory;

	pvMemory = mcQueue.Peek(puiDataSize);
	if (!pvMemory)
	{
		return FALSE;
	}

	if (*puiDataSize <= uiMaxDataSize)
	{
		memcpy(pvData, pvMemory, *puiDataSize);
		mcQueue.Drop(pvMemory);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::IsEmpty(void)
{
	return mcQueue.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CSharedMemoryQueue::GetCacheSize(void)
{
	return mcQueue.GetCacheSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CSharedMemoryQueue::GetAllocatedSize(void)
{
	return mcQueue.GetAllocatedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::ValidateQueue(void)
{
	return mcQueue.ValidateCache();
}

