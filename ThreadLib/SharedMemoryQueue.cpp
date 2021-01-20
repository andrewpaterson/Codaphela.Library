#include "SharedMemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(char* szName, size_t uiByteSize)
{
	return InitMaster(szName, uiByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::Init(char* szName)
{
	return InitClient(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::InitMaster(char* szName, size_t uiByteSize)
{
	BOOL				bResult;
	SSharedMemoryMap	sMap;
	CChars				szMemoryName;
	CChars				szMutexName;

	mbMaster = TRUE;

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();

	bResult = mcMutex.Create();
	if (!bResult)
	{
		mcMutex.Close();
		return FALSE;
	}

	bResult = mcMutex.Lock();
	if (!bResult)
	{
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	bResult = mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
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

	sMap = mcSharedMemory.Touch();
	if (!sMap.pvMemory)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}
	mcQueue.Init(sMap.pvMemory, sMap.uiSize);
	mcMutex.Unlock();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::InitClient(char* szName)
{
	BOOL				bResult;
	SSharedMemoryMap	sMap;
	CChars				szMemoryName;
	CChars				szMutexName;

	mbMaster = FALSE;

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();

	bResult = mcMutex.Connect();
	if (!bResult)
	{
		mcMutex.Close();
		return FALSE;
	}

	bResult = mcMutex.Lock();
	if (!bResult)
	{
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	bResult = mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}

	bResult = mcSharedMemory.Connect();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}

	sMap = mcSharedMemory.Touch();
	if (!sMap.pvMemory)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return FALSE;
	}
	mcQueue.InitExisting((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);
	mcMutex.Unlock();

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
	void*				pvMemory;

	mcMutex.Lock();
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	pvMemory = mcQueue.Push(uiDataSize);
	if (!pvMemory)
	{
		SSharedMemoryMap	sResize = mcSharedMemory.IncreaseSize(uiDataSize);
		if (!sResize.pvMemory)
		{
			mcMutex.Unlock();
			return FALSE;
		}

		//You can't call remap after mcQueue.mpsDetail has been freed.
		mcQueue.Remap((SCircularMemoryList*)sResize.pvMemory, sResize.uiSize);
		pvMemory = mcQueue.Push(uiDataSize);
		if (!pvMemory)
		{
			mcMutex.Unlock();
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
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	pvMemory = mcQueue.Peek(puiDataSize);
	if (!pvMemory)
	{
		mcMutex.Unlock();
		return FALSE;
	}

	if (*puiDataSize <= uiMaxDataSize)
	{
		memcpy(pvData, pvMemory, *puiDataSize);
		mcQueue.Drop(pvMemory);
		mcMutex.Unlock();
		return TRUE;
	}
	else
	{
		mcMutex.Unlock();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSharedMemoryQueue::IsEmpty(void)
{
	BOOL	bEmpty;

	mcMutex.Lock();
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	
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
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

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
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

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
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

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
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	bResult = mcQueue.ValidateCache();
	
	mcMutex.Unlock();
	return bResult;
}

