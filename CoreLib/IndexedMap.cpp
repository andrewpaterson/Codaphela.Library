#include "IndexedCache.h"
#include "IndexedMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iDataCacheSize, EIndexWriteThrough eWriteThrough)
{
	Init(pcDurableFileControl, szDataExtension, szDescricptorName, szDescricptorRewrite, iDataCacheSize, eWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iDataCacheSize, EIndexWriteThrough eWriteThrough, CEvictionCallback* pcEvictionUserCallback)
{
	CIndexedDataCommon::Init(pcEvictionUserCallback);

	meWriteThrough = eWriteThrough;

	pcDurableFileControl->Begin();

	InitIndices(pcDurableFileControl, TRUE);
	mcData.Init(pcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", iDataCacheSize, eWriteThrough, this);

	pcDurableFileControl->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::Kill(void)
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
void CIndexedMap::InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting)
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
void CIndexedMap::NullCachedDescriptors(void)
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
BOOL CIndexedMap::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
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
			bResult &= mpcEvictionCallback->NodeEvicted(&psDesc->oi, sizeof(OIndex), pvCache, psDesc->uiSize);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
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
BOOL CIndexedMap::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
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
BOOL CIndexedMap::UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize)
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
BOOL CIndexedMap::RemoveDescriptor(OIndex oi)
{
	mbDescriptorsWritten = FALSE;
	return mcDescriptors.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::Flush(BOOL bClearCache)
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
int64 CIndexedMap::NumElements(void)
{
	return mcDescriptors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::IsDirty(OIndex oi)
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
BOOL CIndexedMap::EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return GetDescriptor(oi, pcDescriptor);
}

