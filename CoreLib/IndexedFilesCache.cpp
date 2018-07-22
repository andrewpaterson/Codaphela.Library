#include "BaseLib/MemoryCacheAllocation.h"
#include "BaseLib/StackMemory.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedFilesCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesCache::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback)
{
	mbWriteThrough = bWriteThrough;
	mpcEvictionCallback = pcEvictionCallback;

	mcDataFiles.Init(pcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX");
	mcDataFiles.ReadIndexedFileDescriptors();

	if (iCacheSize != 0)
	{
		mcDataCache.Init(iCacheSize);
		mbCaching = TRUE;
	}
	else
	{
		mcDataCache.Zero();
		mbCaching = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesCache::Kill(void)
{
	if (mbCaching)
	{
		mcDataCache.Kill();
	}
	mcDataFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	BOOL	bResult;

	//The descriptor has always been set prior to calling this.
	if (pcDescriptor->IsCached())
	{
		memcpy_fast(pvData, pcDescriptor->GetCache(), pcDescriptor->GetDataSize());
		return TRUE;
	}
	else
	{
		bResult = CacheRead(oi, pcDescriptor);
		if (bResult)
		{
			if (pcDescriptor->IsCached())
			{
				memcpy_fast(pvData, pcDescriptor->GetCache(), pcDescriptor->GetDataSize());
				return mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
			}
			else
			{
				return mcDataFiles.Read(pcDescriptor, pvData);
			}
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::SetData(CIndexedDataDescriptor* pcDescriptor, void * pvData)
{
	BOOL	bUpdated;

	bUpdated = mcDataCache.Update(pcDescriptor, pvData);
	if (bUpdated && mbWriteThrough)
	{
		return mcDataFiles.Write(pcDescriptor, pvData);
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::Flush(BOOL bClearCache)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcDataCache.StartIteration();
		while (psCached)
		{
			bResult = WriteEvictedData(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcDataCache.Iterate(psCached);
		}
		if (bClearCache && !bAnyFailed)
		{
			mcDataCache.Clear();
		}
		return !bAnyFailed;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::Uncache(void)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcDataCache.StartIteration();
		while (psCached)
		{
			bResult = ClearDescriptorCache(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcDataCache.Iterate(psCached);
		}
		mcDataCache.Clear();
		return bAnyFailed;
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
int CIndexedFilesCache::NumCached(void)
{
	if (mbCaching)
	{
		return mcDataCache.NumCached();
	}
	else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFilesCache::NumCached(int iSize)
{
	if (mbCaching)
	{
		return mcDataCache.NumCached(iSize);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFilesCache::NumFiles(void)
{
	return mcDataFiles.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFilesCache::NumData(int iDataSize)
{
	return mcDataFiles.NumData(iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached)
{
	void*	pvData;
	BOOL	bResult;

	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		pvData = RemapSinglePointer(psCached, sizeof(SIndexedCacheDescriptor));
		bResult = mcDataFiles.Write(pcDescriptor, pvData);
		return bResult;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesCache::InvalidateData(CIndexedDataDescriptor* pcDescriptor)
{
	if (mbCaching)
	{
		mcDataCache.Invalidate(pcDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::SetData(CIndexedDataDescriptor* pcDescriptor, void* pvData, OIndex oi, unsigned int uiTimeStamp)
{
	BOOL	bWritten;
	BOOL	bResult;

	bResult = CacheWrite(oi, pcDescriptor, pvData, &bWritten);
	if (bResult)
	{
		if (!bWritten && mbWriteThrough)
		{
			bResult = mcDataFiles.Write(pcDescriptor, pvData);
			return bResult;
		}
		else
		{
			return TRUE;
		}
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
BOOL CIndexedFilesCache::CacheRead(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcDataCache.Allocate(oi, pcDescriptor, &cPreAllocated);
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcDataFiles.Read(pcDescriptor, pcDescriptor->GetCache());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			//This needs to be a callback.
			mpcEvictionCallback->EvictDescriptors(cPreAllocated.GetEvictedArray());
			cPreAllocated.Kill();
			return TRUE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::CacheWrite(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			*pbWritten = FALSE;
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcDataCache.Allocate(oi, pcDescriptor, &cPreAllocated);
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			memcpy_fast(pcDescriptor->GetCache(), pvData, pcDescriptor->GetDataSize());

			mpcEvictionCallback->EvictDescriptors(cPreAllocated.GetEvictedArray());
			cPreAllocated.Kill();
			return TRUE;
		}
		else
		{
			//There wasn't enough space in the cache... the object is written immediately.
			bResult = mcDataFiles.Write(pcDescriptor, pvData);
			*pbWritten = TRUE;

			cPreAllocated.Kill();
			return bResult;
		}
	}
	else
	{
		*pbWritten = TRUE;
		bResult = mcDataFiles.Write(pcDescriptor, pvData);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::EvictFromCache(CIndexedDataDescriptor* pcDescriptor)
{
	SIndexedCacheDescriptor*	psExisting;
	BOOL						bResult;

	if (pcDescriptor->IsCached())
	{
		psExisting = mcDataCache.GetHeader(pcDescriptor->GetCache());
		bResult = WriteEvictedData(pcDescriptor, psExisting);
		mcDataCache.Invalidate(psExisting);
		return bResult;
	}
	else
	{
		return TRUE;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::WriteEvictedData(SIndexedCacheDescriptor* psCached)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = mpcEvictionCallback->GetDescriptor(psCached->oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	//We can assume the cache is clear of the data because it has been evicted.
	cDescriptor.Cache(NULL);

	bResult = WriteEvictedData(&cDescriptor, psCached);
	if (!bResult)
	{
		return FALSE;
	}

	return mpcEvictionCallback->SetDescriptor(psCached->oi, &cDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::WriteEvictedData(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psCached;
	BOOL						bResult;

	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		psCached = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		bResult = WriteEvictedData(psCached);
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesCache::CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	//This function tells the disk whether it must update itself because the cached value has changed.
	//It also timestamps the descriptor of the changed data.
	unsigned int		uiDataSize;
	void*				pvTemp;
	CStackMemory<>		cStack;
	BOOL				bResult;

	if (pcDescriptor->HasFile())
	{
		uiDataSize = pcDescriptor->GetDataSize();
		pvTemp = cStack.Init(uiDataSize);

		mcDataFiles.Read(pcDescriptor, pvTemp);
		bResult = memcmp(pvTemp, pvData, uiDataSize) == 0;
		cStack.Kill();
		return bResult;
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
BOOL CIndexedFilesCache::ClearDescriptorCache(SIndexedCacheDescriptor* psCached)
{
	CIndexedDataDescriptor		cDescriptor;
	BOOL						bResult;

	bResult = mpcEvictionCallback->GetDescriptor(psCached->oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	//We can assume the cache is clear of the data because it has been evicted.
	cDescriptor.Cache(NULL);

	//These can never ever be unequal.  But just in case...
	if (cDescriptor.GetDataSize() == psCached->iDataSize)
	{
		return mpcEvictionCallback->SetDescriptor(psCached->oi, &cDescriptor);
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedFilesCache::TestGetCachedObjectSize(OIndex oi)
{
	SIndexedCacheDescriptor*	psDesc;

	psDesc = mcDataCache.TestGetDescriptor(oi);
	if (psDesc)
	{
		return sizeof(SIndexedCacheDescriptor) + psDesc->iDataSize;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFilesCache::TestNumIgnoredCacheElements(void) { return mcDataCache.NumIgnored(); }
BOOL CIndexedFilesCache::IsCaching(void) { return mbCaching; }

