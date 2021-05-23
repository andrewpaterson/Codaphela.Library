#include "IndexedCache.h"
#include "IndexedMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite, size_t iDataCacheSize, EIndexWriteThrough eWriteThrough)
{
	Init(pcDurableFileControl, szSubDirectory, szDataExtension, szDescriptorName, szDescriptorRewrite, iDataCacheSize, eWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite, size_t iDataCacheSize, EIndexWriteThrough eWriteThrough, CIndexedDataEvictionCallback* pcEvictionUserCallback)
{
	CIndexedDataCommon::Init(pcEvictionUserCallback);

	meWriteThrough = eWriteThrough;

	pcDurableFileControl->Begin();

	InitIndices(pcDurableFileControl, TRUE);
	mcData.Init(pcDurableFileControl, szSubDirectory, "DAT", "Files.IDX", "_Files.IDX", iDataCacheSize, eWriteThrough, this);

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

	mcDescriptorsFile.Kill();
	mcDescriptors.Kill();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting)
{
	mbDescriptorsWritten = TRUE;
	mcDescriptorsFile.Init(pcDurableFileControl, "Descriptors.IDX", "_Descriptors.IDX");

	if (mcDescriptorsFile.Exists())
	{
		mcDescriptors.Read(&mcDescriptorsFile);
		NullCachedDescriptors();
	}
	else
	{
		mcDescriptors.Init(TRUE);
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
	
	bExists = mcDescriptors.StartIteration(&sIter, (void**)&poi, NULL, (void**)&pcDescriptor, NULL);
	while (bExists)
	{
		pcDescriptor->ClearCache();
		bExists = mcDescriptors.Iterate(&sIter, (void**)&poi, NULL, (void**)&pcDescriptor, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMap::DescriptorEvicted(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	if (mpcIndexedDataEvictionCallback)
	{
		return mpcIndexedDataEvictionCallback->IndexEvicted(oi, pvCache, uiDataSize);
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
int64 CIndexedMap::NumIndices(void)
{
	return mcDescriptors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedMap::NumIndicesCached(void)
{
	return NumIndices();
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
			psDataDescriptor = (SIndexedCacheDescriptor*)RemapSinglePointer(pcKeyDescriptor->GetCache(), -(ptrdiff_t)sizeof(SIndexedCacheDescriptor));
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

