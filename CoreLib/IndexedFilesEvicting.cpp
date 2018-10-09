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
BOOL CIndexedFilesEvicting::GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	BOOL	bResult;
	void*	pvCache;

	//The descriptor has always been set prior to calling this.
	pvCache = pcDescriptor->GetCache();
	if (pvCache != NULL)
	{
		memcpy_fast(pvData, pvCache, pcDescriptor->GetDataSize());
		return TRUE;
	}
	else
	{
		if (mbCaching && pcDescriptor->GetDataSize() <= mcDataCache.GetCacheSize())
		{
			bResult = CacheAllocate(oi, pcDescriptor);
			if (bResult)
			{
				pvCache = pcDescriptor->GetCache();
				bResult = mcDataFiles.Read(pcDescriptor, pvCache);
				memcpy_fast(pvData, pvCache, pcDescriptor->GetDataSize());
				mpcEvictionCallback->UpdateDescriptorCache(oi, pvCache);
				return bResult;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return mcDataFiles.Read(pcDescriptor, pvData);
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
			bResult = WriteEvictedData(psCached, bClearCache);
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
	CFileDataIndex	cIndex;
	void*			pvCache;

	cIndex.Init(pcDescriptor->GetFileIndex(), pcDescriptor->GetDataIndexInFile());
	if (mbCaching)
	{
		if (mbWriteThrough)
		{
			mcDataCache.Invalidate(pcDescriptor);
			mcDataFiles.Delete(&cIndex);
		}
		else
		{
			pvCache = pcDescriptor->GetCache();
			if (pvCache)
			{
				mcDataCache.Invalidate(pcDescriptor);
				mcDataFiles.Delete(&cIndex);
			}
			else
			{
				mcDataFiles.Delete(&cIndex);
			}
		}
	}
	else
	{
		mcDataFiles.Delete(&cIndex);
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
	BOOL	bResult;
	void*	pvCache;
	
	if (mbCaching && pcDescriptor->GetDataSize() <= mcDataCache.GetCacheSize())
	{
		pvCache = pcDescriptor->GetCache();
		if (pvCache == NULL) 
		{
			if (!pcDescriptor->HasFile())
			{
				//New data
				pcDescriptor->SetDataSize(uiDataSize);
				bResult = CacheAllocate(oi, pcDescriptor);
				if (!bResult)
				{
					return FALSE;
				}
				pvCache = pcDescriptor->GetCache();

				memcpy_fast(pvCache, pvData, pcDescriptor->GetDataSize());
				if (mbWriteThrough)
				{
					bResult = mcDataFiles.Write(pcDescriptor, pvData);
				}
				else
				{
					mcDataCache.SetDirty(pvCache);
					bResult = TRUE;
				}
				bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
				return bResult;
			}
			else
			{
				//Data not cached.  File exists.
				if (uiDataSize == pcDescriptor->GetDataSize())
				{
					//Descriptor size same as Set size.
					bResult = CacheAllocate(oi, pcDescriptor);
					if (!bResult)
					{
						return FALSE;
					}
					pvCache = pcDescriptor->GetCache();

					memcpy_fast(pvCache, pvData, pcDescriptor->GetDataSize());
					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
				else
				{
					//Descriptor size different to Set size.
					pcDescriptor->SetDataSize(uiDataSize);
					InvalidateData(pcDescriptor);
					bResult = CacheAllocate(oi, pcDescriptor);
					if (!bResult)
					{
						return FALSE;
					}
					pvCache = pcDescriptor->GetCache();

					memcpy_fast(pvCache, pvData, pcDescriptor->GetDataSize());
					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
			}
		}
		else
		{
			if (!pcDescriptor->HasFile())
			{
				//Cached data.  File not written.
				if (uiDataSize == pcDescriptor->GetDataSize())
				{
					//Descriptor size same as Set size.
					memcpy_fast(pvCache, pvData, uiDataSize);
					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
				else
				{
					//Descriptor size different to Set size.
					pcDescriptor->SetDataSize(uiDataSize);
					InvalidateData(pcDescriptor);
					bResult = CacheAllocate(oi, pcDescriptor);
					if (!bResult)
					{
						return FALSE;
					}
					pvCache = pcDescriptor->GetCache();

					memcpy_fast(pvCache, pvData, pcDescriptor->GetDataSize());
					mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);

					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
			}
			else
			{
				//Cached data.  File exists.
				if (uiDataSize == pcDescriptor->GetDataSize())
				{
					//Descriptor size same as Set size.
					memcpy_fast(pvCache, pvData, uiDataSize);

					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
				else
				{
					//Descriptor size different to Set size.
					pcDescriptor->SetDataSize(uiDataSize);
					InvalidateData(pcDescriptor);
					bResult = CacheAllocate(oi, pcDescriptor);
					if (!bResult)
					{
						return FALSE;
					}
					pvCache = pcDescriptor->GetCache();

					memcpy_fast(pvCache, pvData, pcDescriptor->GetDataSize());
					mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);

					if (mbWriteThrough)
					{
						bResult = mcDataFiles.Write(pcDescriptor, pvData);
					}
					else
					{
						mcDataCache.SetDirty(pvCache);
						bResult = TRUE;
					}
					bResult |= mpcEvictionCallback->SetDescriptor(oi, pcDescriptor);
					return bResult;
				}
			}
		}
	}
	else
	{
		bResult = mcDataFiles.Write(pcDescriptor, pvData);
		return bResult;
	}
}
		

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::CacheAllocate(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	cPreAllocated.Init(pcDescriptor->GetDataSize(), mcDataCache.GetIndexCacheDescritorSize());
	if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
	{
		bResult = DescriptorsEvicted(cPreAllocated.GetEvictedArray());
		if (bResult)
		{
			//pcDescriptor cache and size updated in .Allocate.
			bResult = mcDataCache.Allocate(oi, pcDescriptor, &cPreAllocated);
		}
		cPreAllocated.Kill();
		return bResult;
	}
	else
	{
		cPreAllocated.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::DescriptorsEvicted(CArrayVoidPtr* paEvictedCacheDescriptors)
{
	BOOL	bResult;

	bResult = WriteEvictedData(paEvictedCacheDescriptors);
	if (!bResult)
	{
		return FALSE;
	}

	mpcEvictionCallback->DescriptorsEvicted(paEvictedCacheDescriptors);
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
		pvData = pcDescriptor->GetCache();
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
	BOOL						bResult;
	void*						pvData;
	SIndexedCacheDescriptor*	psDescriptor;

	pvData = pcDescriptor->GetCache();
	psDescriptor = mcDataCache.GetHeader(pvData);
	if (psDescriptor->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		if (bClearCache)
		{
			pcDescriptor->Cache(NULL);
		}
		psDescriptor->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		bResult = mcDataFiles.Write(pcDescriptor, pvData);  //Be careful of this write.  It doesn't work if the data size has changed.
		if (!bResult)
		{
			return FALSE;
		}

		return mpcEvictionCallback->SetDescriptor(psDescriptor->oi, pcDescriptor, TRUE);
	}
	else
	{
		if (bClearCache)
		{
			return mpcEvictionCallback->UpdateDescriptorCache(psDescriptor->oi, NULL);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::WriteEvictedData(SIndexedCacheDescriptor* psCached, BOOL bClearCache)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;
	void*					pvData;

	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		bResult = mpcEvictionCallback->GetDescriptor(psCached->oi, &cDescriptor);
		if (!bResult)
		{
			return FALSE;
		}

		if (bClearCache)
		{
			cDescriptor.Cache(NULL);
		}
		psCached->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		pvData = RemapSinglePointer(psCached, sizeof(SIndexedCacheDescriptor));
		bResult = mcDataFiles.Write(&cDescriptor, pvData);  //Be careful of this write.  It doesn't work if the data size has changed.
		if (!bResult)
		{
			return FALSE;
		}

		return mpcEvictionCallback->SetDescriptor(psCached->oi, &cDescriptor);
	}
	else
	{
		if (bClearCache)
		{
			return mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::WriteEvictedData(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psCached;
	BOOL						bResult;

	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		psCached = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		bResult = WriteEvictedData(psCached, TRUE);
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
BOOL CIndexedFilesEvicting::CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData)
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
BOOL CIndexedFilesEvicting::ClearDescriptorCache(SIndexedCacheDescriptor* psCached)
{
	return mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL);
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

