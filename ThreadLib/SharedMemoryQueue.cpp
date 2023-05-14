#include "SharedMemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::Init(char* szName, size_t uiByteSize)
{
	return InitMaster(szName, uiByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::Init(char* szName)
{
	return InitClient(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::InitMaster(char* szName, size_t uiByteSize)
{
	bool				bResult;
	SSharedMemoryMap	sMap;
	CChars				szMemoryName;
	CChars				szMutexName;

	mbMaster = true;

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();

	bResult = mcMutex.Create();
	if (!bResult)
	{
		mcMutex.Close();
		return false;
	}

	bResult = mcMutex.Lock();
	if (!bResult)
	{
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	bResult = mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	bResult = mcSharedMemory.Create(uiByteSize + sizeof(SCircularMemoryList));
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	sMap = mcSharedMemory.Touch();
	if (!sMap.pvMemory)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}
	mcQueue.Init(sMap.pvMemory, sMap.uiSize);
	mcMutex.Unlock();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::InitClient(char* szName)
{
	bool				bResult;
	SSharedMemoryMap	sMap;
	CChars				szMemoryName;
	CChars				szMutexName;

	mbMaster = false;

	szMutexName.Init("Local\\ProcessMutex:")->Append(szName);
	mcMutex.Init(szMutexName.Text());
	szMutexName.Kill();

	bResult = mcMutex.Connect();
	if (!bResult)
	{
		mcMutex.Close();
		return false;
	}

	bResult = mcMutex.Lock();
	if (!bResult)
	{
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	szMemoryName.Init("Local\\SharedMemory:")->Append(szName);
	bResult = mcSharedMemory.Init(szMemoryName.Text());
	szMemoryName.Kill();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	bResult = mcSharedMemory.Connect();
	if (!bResult)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}

	sMap = mcSharedMemory.Touch();
	if (!sMap.pvMemory)
	{
		mcSharedMemory.Close();
		mcMutex.Unlock();
		mcMutex.Close();
		return false;
	}
	mcQueue.InitExisting((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);
	mcMutex.Unlock();

	return true;
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
bool CSharedMemoryQueue::Push(void* pvData, size_t uiDataSize)
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
			return false;
		}

		mcQueue.Remap((SCircularMemoryList*)sResize.pvMemory, sResize.uiSize);
		pvMemory = mcQueue.Push(uiDataSize);
		if (!pvMemory)
		{
			mcMutex.Unlock();
			return false;
		}
	}

	memcpy(pvMemory, pvData, uiDataSize);
	mcMutex.Unlock();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::Pop(void* pvData, size_t* puiDataSize, size_t uiMaxDataSize)
{
	void*	pvMemory;

	mcMutex.Lock();
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	pvMemory = mcQueue.Peek(puiDataSize);
	if (!pvMemory)
	{
		mcMutex.Unlock();
		return false;
	}

	if (*puiDataSize <= uiMaxDataSize)
	{
		memcpy(pvData, pvMemory, *puiDataSize);
		mcQueue.Drop(pvMemory);
		mcMutex.Unlock();
		return true;
	}
	else
	{
		mcMutex.Unlock();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedMemoryQueue::IsEmpty(void)
{
	bool	bEmpty;

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
bool CSharedMemoryQueue::ValidateQueue(void)
{
	bool	bResult;

	mcMutex.Lock();
	SSharedMemoryMap sMap = mcSharedMemory.Touch();
	mcQueue.Remap((SCircularMemoryList*)sMap.pvMemory, sMap.uiSize);

	bResult = mcQueue.ValidateCache();
	
	mcMutex.Unlock();
	return bResult;
}

