#include "IndexedCache.h"
#include "IndexedFilesEvictedDescriptorList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iDataCacheSize, BOOL bWriteThrough)
{
	Init(pcDurableFileControl, szDataExtension, szDescricptorName, szDescricptorRewrite, iDataCacheSize, bWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iDataCacheSize, BOOL bWriteThrough, CEvictionCallback* pcEvictionUserCallback)
{
	CIndexedDataCommon::Init(pcEvictionUserCallback);

	mbWriteThrough = bWriteThrough;

	pcDurableFileControl->Begin();

	InitIndices(pcDurableFileControl, TRUE);
	mcData.Init(pcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", iDataCacheSize, bWriteThrough, this);

	pcDurableFileControl->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::Kill(void)
{
	if (!mcData.IsFlushed())
	{
		return FALSE;
	}
	if (!mbDescriptorsWritten)
	{
		return FALSE;
	}

	mcData.Kill();

	mcDescriptors.Kill();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting)
{
	mcDescriptors.Init(TRUE);
	mbDescriptorsWritten = TRUE;
	pcDurableFileControl->InitFile(&mcDescriptorsFile, "Descriptors.IDX", "_Descriptors.IDX");

	if (mcDescriptorsFile.Exists())
	{
		mcDescriptors.Read(&mcDescriptorsFile);
		NullCachedDescriptors();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::NullCachedDescriptors(void)
{
	SMapIterator			sIter;
	OIndex*					poi;
	CIndexedDataDescriptor*	pcDescriptor;
	BOOL					bExists;
	
	bExists = mcDescriptors.StartIteration(&sIter, (void**)&poi, (void**)&pcDescriptor);
	while (bExists)
	{
		pcDescriptor->ClearCache();
		bExists = mcDescriptors.Iterate(&sIter, (void**)&poi, (void**)&pcDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	BOOL						bResult;
	int							iNumElements;
	void*						pvCache;

	bResult = TRUE;
	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			pvCache = mcData.GetCachedData(psDesc);
			bResult &= mpcEvictionCallback->NodeEvicted(&psDesc->oi, sizeof(OIndex), pvCache, psDesc->iDataSize);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
{
	CIndexedDataDescriptor* pcResult;

	pcResult = mcDescriptors.Get(oi);
	if (pcResult)
	{
		if (pcDescriptor)
		{
			memcpy_fast(pcDescriptor, pcResult, sizeof(CIndexedDataDescriptor));
		}
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
BOOL CIndexedFilesEvictedDescriptorList::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
{
	CIndexedDataDescriptor* pcExistingDescriptor;
	BOOL					bUpdated;

	pcExistingDescriptor = mcDescriptors.Get(oi);
	if (!pcExistingDescriptor)
	{
		mbDescriptorsWritten = FALSE;
		return mcDescriptors.Put(oi, pcDescriptor);
	}
	else
	{
		bUpdated = pcExistingDescriptor->Update(pcDescriptor);
		if (bUpdated)
		{
			mbDescriptorsWritten = FALSE;
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	CIndexedDataDescriptor* pcDescriptor;

	pcDescriptor = mcDescriptors.Get(oi);
	if (pcDescriptor)
	{
		pcDescriptor->Cache(pvCache, uiDataSize);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::RemoveDescriptor(OIndex oi)
{
	mbDescriptorsWritten = FALSE;
	return mcDescriptors.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::Flush(BOOL bClearCache)
{
	BOOL		bResult;

	bResult = mcData.Flush(bClearCache);
	if (!mbDescriptorsWritten)
	{
		bResult &= mcDescriptors.Write(&mcDescriptorsFile);
		mbDescriptorsWritten = TRUE;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFilesEvictedDescriptorList::NumElements(void)
{
	return mcDescriptors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::IsDirty(OIndex oi)
{
	CIndexedDataDescriptor*		pcKeyDescriptor;
	SIndexedCacheDescriptor*	psDataDescriptor;

	pcKeyDescriptor = mcDescriptors.Get(oi);
	if (pcKeyDescriptor)
	{
		if (pcKeyDescriptor->GetCache())
		{
			psDataDescriptor = (SIndexedCacheDescriptor*)RemapSinglePointer(pcKeyDescriptor->GetCache(), -(int)(sizeof(SIndexedCacheDescriptor)));
			if (psDataDescriptor->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return GetDescriptor(oi, pcDescriptor);
}

