#include "SharedMemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(char* szName, size_t uiByteSize)
{
	mbMaster = TRUE;
	InitNames(szName);

	return InitMaster(uiByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(char* szName)
{
	BOOL	bResult;

	mbMaster = FALSE;
	InitNames(szName);

	mcMutex.Connect();
	mcMutex.Lock();
	bResult = InitClient();
	mcMutex.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedMemoryQueue::InitNames(char* szName)
{
	CChars		szMemoryName;
	CChars		szMutexName;

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::InitMaster(size_t uiByteSize)
{
	BOOL	bResult;
	void*	pvMemory;

	bResult = mcMutex.Create();
	if (!bResult)
	{
		mcMutex.Close();
		return FALSE;
	}

	bResult = mcMutex.Lock();
	if (!bResult)
	{
		mcMutex.Close();
		return FALSE;
	}

	bResult = mcSharedMemory.Create(uiByteSize + sizeof(SCircularMemoryList));
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}

	pvMemory = mcSharedMemory.Touch();
	if (!pvMemory)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}
	mcQueue.Init(pvMemory, mcSharedMemory.GetSize());
	mcMutex.Unlock();

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
	mcMutex.Close();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Push(void* pvData, size_t uiDataSize)
{
	void*	pvMemory;

	mcMutex.Lock();

	pvMemory = mcQueue.Push(uiDataSize);
	if (!pvMemory)
	{
		SSharedMemoryResize	sResize = mcSharedMemory.IncreaseSize(uiDataSize);
		if (!sResize.pvMemory)
		{
			return FALSE;
		}

		//You can't call remap after mcQueue.mpsDetail has been freed.
		mcQueue.Remap((SCircularMemoryList*)sResize.pvMemory, sResize.uiSize);
		pvMemory = mcQueue.Push(uiDataSize);
		if (!pvMemory)
		{
			return FALSE;
		}
	}

	memcpy(pvMemory, pvData, uiDataSize);
	mcMutex.Unlock();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Pop(void* pvData, size_t* puiDataSize, size_t uiMaxDataSize)
{
	void*	pvMemory;

	mcMutex.Lock();
	
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
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::IsEmpty(void)
{
	BOOL	bEmpty;

	mcMutex.Lock();
	bEmpty = mcQueue.IsEmpty();
	mcMutex.Unlock();

	return bEmpty;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSharedMemoryQueue::NumElements(void)
{
	int		i;

	mcMutex.Lock();
	i = mcQueue.NumElements();
	mcMutex.Unlock();

	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CSharedMemoryQueue::GetCacheSize(void)
{
	size_t uiCacheSize;

	mcMutex.Lock();
	uiCacheSize = mcQueue.GetCacheSize();
	mcMutex.Unlock();

	return uiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CSharedMemoryQueue::GetAllocatedSize(void)
{
	size_t uiAllocatedSize;

	mcMutex.Lock();
	uiAllocatedSize = mcQueue.GetAllocatedSize();
	mcMutex.Unlock();

	return uiAllocatedSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::ValidateQueue(void)
{
	BOOL	bResult;

	mcMutex.Lock();
	bResult = mcQueue.ValidateCache();
	mcMutex.Unlock();

	return bResult;
}

