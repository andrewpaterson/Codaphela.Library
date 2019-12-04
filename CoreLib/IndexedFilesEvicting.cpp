#include "BaseLib/MemoryCacheAllocation.h"
#include "BaseLib/StackMemory.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedFilesEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvicting::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback)
{
	mbWriteThrough = bWriteThrough;
	mpcEvictionCallback = pcEvictionCallback;

	mcDataFiles.Init(pcDurableFileControl, szDataExtension, szDescricptorName, szDescricptorRewrite);
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
void CIndexedFilesEvicting::Kill(void)
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
BOOL CIndexedFilesEvicting::GetData(OIndex oi, CIndexedDataDescriptor* pcExistingDescriptor, void* pvData)
{
	BOOL					bResult;
	void*					pvCache;
	CIndexedCacheResult		cResult;
	unsigned int			uiDataSize;
	CFileDataIndex			cDataIndex;

	//The descriptor has always been set prior to calling this.

	pvCache = pcExistingDescriptor->GetCache();
	if (pvCache != NULL)
	{
		//Data already in cache so just copy it into pvData.  Assume pvData is large enough.
		uiDataSize = pcExistingDescriptor->GetCacheDataSize();
		memcpy_fast(pvData, pvCache, uiDataSize);
		return TRUE;
	}
	else
	{
		//Data not in cache.
		uiDataSize = pcExistingDescriptor->GetFileDataSize();
		if (mbCaching && uiDataSize <= mcDataCache.GetCacheSize())
		{
			cResult = CacheAllocate(oi, uiDataSize);
			if (cResult.IsCached())
			{
				pvCache = cResult.GetCache();
				cDataIndex = pcExistingDescriptor->GetFileDataIndex();
				bResult = mcDataFiles.Read(&cDataIndex, pvCache);
				if (bResult)
				{
					memcpy_fast(pvData, pvCache, uiDataSize);
					mpcEvictionCallback->UpdateDescriptorCache(oi, pvCache, uiDataSize);
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			cDataIndex = pcExistingDescriptor->GetFileDataIndex();
			return mcDataFiles.Read(&cDataIndex, pvData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::Flush(BOOL bClearCache)
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
			bResult = WriteEvictedData(psCached, bClearCache, TRUE);  
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
BOOL CIndexedFilesEvicting::IsFlushed(void)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcDataCache.StartIteration();
		while (psCached)
		{
			if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
			{
				return FALSE;
			}
			psCached = mcDataCache.Iterate(psCached);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFilesEvicting::NumCached(void)
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
int CIndexedFilesEvicting::NumCached(int iSize)
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
int CIndexedFilesEvicting::NumFiles(void)
{
	return mcDataFiles.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFilesEvicting::NumData(int iDataSize)
{
	return mcDataFiles.NumData(iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvicting::InvalidateData(CIndexedDataDescriptor* pcDescriptor)
{
	CFileDataIndex	cDataIndex;
	void*			pvCache;

	cDataIndex = pcDescriptor->GetFileDataIndex();
	if (mbCaching)
	{
		if (mbWriteThrough)
		{
			mcDataCache.Invalidate(pcDescriptor);
			mcDataFiles.Delete(&cDataIndex);
		}
		else
		{
			pvCache = pcDescriptor->GetCache();
			if (pvCache)
			{
				mcDataCache.Invalidate(pcDescriptor);
				mcDataFiles.Delete(&cDataIndex);
			}
			else
			{
				mcDataFiles.Delete(&cDataIndex);
			}
		}
	}
	else
	{
		mcDataFiles.Delete(&cDataIndex);
	}
}


// X. No cache, no file
// X. Cache same size, no file
// X. Cache diff size, no file
// X. No cache, file same size
// X. No cache, file diff size
// X. Cache same size, file
// X. Cache diff size, file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL					bResult;
	void*					pvNewCache;
	CIndexedDataDescriptor	cResultDescriptor;
	CFilePosIndex			cPosIndex;
	
	if (mbCaching && uiDataSize <= mcDataCache.GetCacheSize())
	{
		pvNewCache = SetCacheData(oi, pcDescriptor, pvData, uiDataSize);
		bResult = pvNewCache != NULL;

		if (mbWriteThrough)
		{
			cPosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
			bResult |= cPosIndex.HasFile();
		}
		else
		{
			mcDataCache.SetDirty(pvNewCache);

			if (pcDescriptor != NULL)
			{
				cPosIndex = *pcDescriptor->GetFilePosIndex();
			}
			else
			{
				cPosIndex.Init();
			}
		}
	}
	else
	{
		//Still need to invalidate any cached data...


		cPosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
		bResult = cPosIndex.HasFile();
	}

	cResultDescriptor.Init(uiDataSize, &cPosIndex, pvNewCache);
	bResult |= mpcEvictionCallback->SetDescriptor(oi, &cResultDescriptor);
	return bResult;
}
		

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedFilesEvicting::SetCacheData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize)
{
	void*					pvExistingCache;
	void*					pvNewCache;
	CIndexedCacheResult		cCacheResult;

	if (pcDescriptor != NULL)
	{
		pvExistingCache = pcDescriptor->GetCache();
	}
	else
	{
		pvExistingCache = NULL;
	}

	//Data not cached.  
	if (pvExistingCache == NULL)
	{
		//Data not cached.  File not written.  (New Data).
		cCacheResult = CacheAllocate(oi, uiDataSize);
		if (!cCacheResult.IsCached())
		{
			return NULL;
		}
		pvNewCache = cCacheResult.GetCache();
		memcpy_fast(pvNewCache, pvData, uiDataSize);
		return pvNewCache;
	}
	else
	{
		if (uiDataSize == pcDescriptor->GetCacheDataSize())
		{
			//Descriptor size same as Set size.  Cached data.  File exists.
			memcpy_fast(pvExistingCache, pvData, uiDataSize);
			return pvExistingCache;
		}
		else
		{
			cCacheResult = CacheAllocate(oi, uiDataSize);
			if (!cCacheResult.IsCached())
			{
				return NULL;
			}

			pvNewCache = cCacheResult.GetCache();
			memcpy_fast(pvNewCache, pvData, uiDataSize);
			mcDataCache.Invalidate(pvExistingCache);
			return pvNewCache;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFilePosIndex CIndexedFilesEvicting::WriteThroughData(CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize)
{
	CFileDataIndex			cDataIndex;
	CFileDataIndex			cNewDataIndex;
	CFilePosIndex			cPosIndex;

	if ((pcDescriptor == NULL) || (!pcDescriptor->HasFile()))
	{
		//File not written.
		cDataIndex = mcDataFiles.WriteNew(pvData, uiDataSize);
		cPosIndex = cDataIndex.ToFilePosIndex(uiDataSize);
		return cPosIndex;
	}
	else
	{
		//File exists.
		if (uiDataSize == pcDescriptor->GetFileDataSize())
		{
			//Descriptor size same as Set size.   File exists.
			cDataIndex = pcDescriptor->GetFileDataIndex();
			cPosIndex = cDataIndex.ToFilePosIndex(uiDataSize);
			return cPosIndex;
		}
		else
		{
			//Descriptor size different to Set size.  File exists.
			cDataIndex = pcDescriptor->GetFileDataIndex();
			cNewDataIndex = mcDataFiles.WriteNew(pvData, uiDataSize);
			cPosIndex = cNewDataIndex.ToFilePosIndex(uiDataSize);
			mcDataFiles.Delete(&cDataIndex);
			return cPosIndex;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedCacheResult CIndexedFilesEvicting::CacheAllocate(OIndex oi, unsigned int uiDataSize)
{
	CIndexedCacheResult		cResult;
	CMemoryCacheAllocation	cPreAllocated;
	BOOL					bResult;

	cPreAllocated.Init(uiDataSize, mcDataCache.GetIndexCacheDescritorSize());
	if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
	{
		bResult = DescriptorsEvicted(cPreAllocated.GetEvictedArray());
		if (bResult)
		{
			cResult = mcDataCache.Allocate(oi, uiDataSize, &cPreAllocated);
		}
		cPreAllocated.Kill();
		return cResult;
	}
	else
	{
		cPreAllocated.Kill();
		cResult.Fail();
		return cResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	BOOL						bResult;
	int							i;
	SIndexedCacheDescriptor*	psCached;

	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		psCached = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		bResult = WriteEvictedData(psCached, TRUE, FALSE);
		if (!bResult)
		{
			return FALSE;
		}
	}

	mpcEvictionCallback->DescriptorsEvicted(papsEvictedIndexedCacheDescriptors);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::Evict(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	void*						pvData;
	BOOL						bResult;
	SIndexedCacheDescriptor*	psDescriptor;

	pvData = pcDescriptor->GetCache();
	if (pvData)
	{
		psDescriptor = mcDataCache.GetHeader(pvData);

		bResult = WriteEvictedData(pcDescriptor, FALSE);
		mcDataCache.Deallocate(psDescriptor);
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
BOOL CIndexedFilesEvicting::WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, BOOL bClearCache)
{
	void*						pvData;
	SIndexedCacheDescriptor*	psCached;
	BOOL						bResult;
	
	pvData = pcDescriptor->GetCache();
	psCached = mcDataCache.GetHeader(pvData);
	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		bResult = WriteEvictedData2(pcDescriptor, psCached->oi, pvData, bClearCache, TRUE);
		psCached->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		return bResult;
	}
	else
	{
		if (bClearCache)
		{
			return mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL, 0);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::WriteEvictedData(SIndexedCacheDescriptor* psCached, BOOL bClearCache, BOOL bNoEviction)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;
	void*					pvData;

	pvData = RemapSinglePointer(psCached, sizeof(SIndexedCacheDescriptor));
	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		bResult = mpcEvictionCallback->GetDescriptor(psCached->oi, &cDescriptor, bNoEviction);
		if (!bResult)
		{
			return FALSE;
		}

		bResult = WriteEvictedData2(&cDescriptor, psCached->oi, pvData, bClearCache, bNoEviction);
		psCached->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		return bResult;

	}
	else
	{
		if (bClearCache)
		{
			return mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL, 0);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::WriteEvictedData2(CIndexedDataDescriptor* pcDescriptor, OIndex oi, void* pvData, BOOL bClearCache, BOOL bNoEviction)
{
	CFilePosIndex			cFilePosIndex;
	CIndexedDataDescriptor	cResultDescriptor;
	unsigned int			uiDataSize;

	uiDataSize = pcDescriptor->GetCacheDataSize();
	if (bClearCache)
	{
		//mcDataCache.Invalidate(pcDescriptor->GetCache());
		pcDescriptor->ClearCache();
	}

	cFilePosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
	if (!cFilePosIndex.HasFile())
	{
		return FALSE;
	}

	cResultDescriptor.Init(uiDataSize, &cFilePosIndex, pcDescriptor->GetCache());
	return mpcEvictionCallback->SetDescriptor(oi, &cResultDescriptor, bNoEviction);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	//This function tells the disk whether it must update itself because the cached value has changed.
	//It also timestamps the descriptor of the changed data.
	unsigned int		uiDataSize;
	void*				pvTemp;
	CStackMemory<>		cStack;
	BOOL				bResult;
	CFileDataIndex		cDataIndex;

	if (pcDescriptor->HasFile())
	{
		uiDataSize = pcDescriptor->GetFileDataSize();
		pvTemp = cStack.Init(uiDataSize);

		cDataIndex = pcDescriptor->GetFileDataIndex();
		mcDataFiles.Read(&cDataIndex, pvTemp);
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
BOOL CIndexedFilesEvicting::ClearDescriptorCache(SIndexedCacheDescriptor* psCached)
{
	return mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedFilesEvicting::GetCachedDataSize(CIndexedDataDescriptor* pcDescriptor)
{
	void*						pvData;
	SIndexedCacheDescriptor*	psDescriptor;

	pvData = pcDescriptor->GetCache();
	if (pvData)
	{
		psDescriptor = mcDataCache.GetHeader(pvData);
		return psDescriptor->iDataSize;
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
unsigned int CIndexedFilesEvicting::TestGetCachedObjectSize(OIndex oi)
{
	SIndexedCacheDescriptor*	psDescriptor;

	psDescriptor = mcDataCache.TestGetDescriptor(oi);
	if (psDescriptor)
	{
		return sizeof(SIndexedCacheDescriptor) + psDescriptor->iDataSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexedFilesEvicting::GetSystemMemorySize(void)
{
	return mcDataCache.GetAllocatedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::IsCaching(void) { return mbCaching; }

