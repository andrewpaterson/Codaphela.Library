#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/MemoryCacheAllocation.h"
#include "BaseLib/StackMemory.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedFilesEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite, size_t iCacheSize, EIndexWriteThrough eWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback)
{
	meWriteThrough = eWriteThrough;
	mpcEvictionCallback = pcEvictionCallback;

	mcDataFiles.Init(pcDurableFileControl, szSubDirectory, szDataExtension, szDescriptorName, szDescriptorRewrite);
	mcDataFiles.ReadIndexedFileDescriptors();

	if (iCacheSize != 0)
	{
		mcDataCache.Init(iCacheSize, this);
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
BOOL CIndexedFilesEvicting::Kill(void)
{
	if (mbCaching)
	{
		if (!IsFlushed())
		{
			return gcLogger.Error2(__METHOD__, " Flush must called before Kill.", NULL);
		}
		mcDataCache.Kill();
	}
	mcDataFiles.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::GetData(OIndex oi, CIndexedDataDescriptor* pcExistingDescriptor, void* pvData, unsigned int uiMaxDataSize)
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
		memcpy_fast(pvData, pvCache, MinDataSize(uiDataSize, uiMaxDataSize));
		return TRUE;
	}
	else
	{
		//Data not in cache.
		uiDataSize = pcExistingDescriptor->GetFileDataSize();
		if (mbCaching && mcDataCache.CanCache(uiDataSize))
		{
			cResult = CacheAllocate(oi, uiDataSize);
			if (cResult.IsCached())
			{
				pvCache = cResult.GetCache();
				cDataIndex = pcExistingDescriptor->GetFileDataIndex();
				bResult = mcDataFiles.Read(&cDataIndex, pvCache);
				if (bResult)
				{
					memcpy_fast(pvData, pvCache, MinDataSize(uiDataSize, uiMaxDataSize));
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
unsigned int CIndexedFilesEvicting::MinDataSize(unsigned int uiDataSize, unsigned int uiMaxDataSize)
{
	if (uiMaxDataSize == 0)
	{
		return uiDataSize;
	}
	else if (uiMaxDataSize > uiDataSize)
	{
		return uiDataSize;
	}
	else
	{
		return uiMaxDataSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::Flush(BOOL bClearCache)
{
	BOOL						bAnyFailed;
	BOOL						bResult;
	EClearCache					eClearCache;
	void*						pvData;
	SIndexedCacheDescriptor*	psDescriptor;

	if (mbCaching)
	{
		if (bClearCache)
		{
			eClearCache = CC_YesDontInvalidate;
		}
		else
		{
			eClearCache = CC_No;
		}

		bAnyFailed = FALSE;
		pvData = mcDataCache.StartIteration();
		while (pvData)
		{
			psDescriptor = mcDataCache.GetDescriptor(pvData);
			bResult = WriteEvictedData1b(psDescriptor, pvData, eClearCache, TRUE);
			if (bResult)
			{
				mpcEvictionCallback->DescriptorEvicted(psDescriptor->oi, pvData, psDescriptor->uiSize);
			}
			else
			{
				bAnyFailed = TRUE;
			}

			pvData = mcDataCache.Iterate(pvData);
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
	void*						pvData;
	SIndexedCacheDescriptor*	psDescriptor;
	BOOL						bAnyFailed;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		pvData = mcDataCache.StartIteration();
		while (pvData)
		{
			psDescriptor = mcDataCache.GetDescriptor(pvData);
			if (psDescriptor->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
			{
				return FALSE;
			}
			pvData = mcDataCache.Iterate(pvData);
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
int64 CIndexedFilesEvicting::NumDatas(void)
{
	return mcDataFiles.NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFilesEvicting::NumDatas(int iDataSize)
{
	return mcDataFiles.NumDatas(iDataSize);
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
		if (meWriteThrough == IWT_Yes)
		{
			mcDataCache.Invalidate(pcDescriptor->GetCache());
			mcDataFiles.Delete(&cDataIndex);
		}
		else if (meWriteThrough == IWT_No)
		{
			pvCache = pcDescriptor->GetCache();
			if (pvCache)
			{
				mcDataCache.Invalidate(pcDescriptor->GetCache());
				mcDataFiles.Delete(&cDataIndex);
			}
			else
			{
				mcDataFiles.Delete(&cDataIndex);
			}
		}
		else 
		{
			gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
			return;
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
BOOL CIndexedFilesEvicting::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize)
{
	BOOL					bResult;
	void*					pvNewCache;
	CIndexedDataDescriptor	cResultDescriptor;
	CFilePosIndex			cPosIndex;
	unsigned int			uiFileDataSize;
	
	if (mbCaching && mcDataCache.CanCache(uiDataSize))
	{
		pvNewCache = SetCacheData(oi, pcDescriptor, pvData, uiDataSize);
		bResult = pvNewCache != NULL;

		if (meWriteThrough == IWT_Yes)
		{
			cPosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
			uiFileDataSize = uiDataSize;
			bResult |= cPosIndex.HasFile();
		}
		else if (meWriteThrough == IWT_No)
		{
			if (pvNewCache != NULL)
			{
				mcDataCache.SetDirty(pvNewCache);
			}

			if (pcDescriptor != NULL)
			{
				cPosIndex = *pcDescriptor->GetFilePosIndex();
				uiFileDataSize = pcDescriptor->GetFileDataSize();
			}
			else
			{
				cPosIndex.Init();
				uiFileDataSize = 0;
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Don't know how to set data with Write Through [IWT_Unknown].", NULL);
			return FALSE;
		}
	}
	else
	{
		if (pcDescriptor != NULL)
		{
			mcDataCache.Invalidate(pcDescriptor->GetCache());
		}
		pvNewCache = NULL;

		cPosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
		uiFileDataSize = uiDataSize;
		bResult = cPosIndex.HasFile();
	}

	cResultDescriptor.Init(uiFileDataSize, &cPosIndex, uiDataSize, pvNewCache);
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
			mcDataFiles.WriteExisting(&cDataIndex, pvData, uiDataSize);
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
BOOL CIndexedFilesEvicting::CacheDataEvicted(void* pvData, SMemoryCacheDescriptor* psDescriptor)
{
	BOOL						bResult;
	SIndexedCacheDescriptor*	psIndexedDescriptor;

	psIndexedDescriptor = (SIndexedCacheDescriptor*)psDescriptor;
	bResult = WriteEvictedData1b(psIndexedDescriptor, pvData, CC_Yes, FALSE);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mpcEvictionCallback->DescriptorEvicted(psIndexedDescriptor->oi, pvData, psIndexedDescriptor->uiSize);
	if (!bResult)
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedCacheResult CIndexedFilesEvicting::CacheAllocate(OIndex oi, unsigned int uiDataSize)
{
	CIndexedCacheResult		cResult;

	cResult = mcDataCache.Allocate(oi, uiDataSize);
	return cResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvicting::Evict(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	void*			pvData;
	BOOL			bResult;
	unsigned int	uiDataSize;

	pvData = pcDescriptor->GetCache();
	if (pvData)
	{
		uiDataSize = pcDescriptor->GetDataSize();
		bResult = EvictWriteData(pcDescriptor, CC_Yes);
		if (bResult)
		{
			bResult &= mpcEvictionCallback->DescriptorEvicted(oi, pvData, uiDataSize);
		}
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
BOOL CIndexedFilesEvicting::EvictWriteData(CIndexedDataDescriptor* pcDescriptor, EClearCache eClearCache)
{
	void*						pvData;
	SIndexedCacheDescriptor*	psCached;
	BOOL						bResult;

	pvData = pcDescriptor->GetCache();
	psCached = mcDataCache.GetDescriptor(pvData);
	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		bResult = WriteEvictedData2(pcDescriptor, psCached->oi, pvData, eClearCache, TRUE);
		psCached->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		return bResult;
	}
	else
	{
		if (eClearCache == CC_Yes)
		{
			bResult = mpcEvictionCallback->UpdateDescriptorCache(psCached->oi, NULL, 0);
			mcDataCache.Invalidate(pvData);
			return bResult;
		}
		else if (eClearCache == CC_YesDontInvalidate)
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
BOOL CIndexedFilesEvicting::WriteEvictedData1b(SIndexedCacheDescriptor* psCached, void* pvData, EClearCache eClearCache, BOOL bNoEviction)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		bResult = mpcEvictionCallback->GetDescriptor(psCached->oi, &cDescriptor, bNoEviction);
		if (!bResult)
		{
			return FALSE;
		}

		bResult = WriteEvictedData2(&cDescriptor, psCached->oi, pvData, eClearCache, bNoEviction);
		psCached->iFlags &= ~CACHE_DESCRIPTOR_FLAG_DIRTY;
		return bResult;
	}
	else
	{
		if ((eClearCache == CC_Yes) || eClearCache == CC_YesDontInvalidate)
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
BOOL CIndexedFilesEvicting::WriteEvictedData2(CIndexedDataDescriptor* pcDescriptor, OIndex oi, void* pvData, EClearCache eClearCache, BOOL bNoEviction)
{
	CFilePosIndex			cFilePosIndex;
	CIndexedDataDescriptor	cResultDescriptor;
	unsigned int			uiDataSize;

	uiDataSize = pcDescriptor->GetCacheDataSize();
	if (uiDataSize == 0)
	{
		return gcLogger.Error2(__METHOD__, " Cannot write evicted data with index [", IndexToString(oi), "] with [0] size.", NULL);
	}

	if (eClearCache == CC_Yes)
	{
		mcDataCache.Invalidate(pcDescriptor->GetCache());
		pcDescriptor->ClearCache();
	}
	else if (eClearCache == CC_YesDontInvalidate)
	{
		pcDescriptor->ClearCache();
	}

	cFilePosIndex = WriteThroughData(pcDescriptor, pvData, uiDataSize);
	if (!cFilePosIndex.HasFile())
	{
		return FALSE;
	}

	cResultDescriptor.Init(uiDataSize, &cFilePosIndex, pcDescriptor->GetCacheDataSize(), pcDescriptor->GetCache());
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
unsigned int CIndexedFilesEvicting::TestGetCachedObjectSize(OIndex oi)
{
	SIndexedCacheDescriptor*	psDescriptor;
	void*						pvData;

	pvData = mcDataCache.TestGetDescriptor(oi);
	if (pvData)
	{
		psDescriptor = mcDataCache.GetDescriptor(pvData);
		return sizeof(SIndexedCacheDescriptor) + psDescriptor->uiSize;
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
BOOL CIndexedFilesEvicting::IsWriteThrough(void) { return meWriteThrough == IWT_Yes; }

