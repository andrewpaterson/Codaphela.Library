#include "IndexedCache.h"
#include "IndexedMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite, size iDataCacheSize, EIndexWriteThrough eWriteThrough)
{
	Init(pcDurableFileControl, szSubDirectory, szDataExtension, szDescriptorName, szDescriptorRewrite, iDataCacheSize, eWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite, size iDataCacheSize, EIndexWriteThrough eWriteThrough, CIndexedDataEvictionCallback* pcEvictionUserCallback)
{
	CIndexedDataCommon::Init(pcEvictionUserCallback);

	meWriteThrough = eWriteThrough;

	pcDurableFileControl->Begin();

	InitIndices(pcDurableFileControl, true);
	mcData.Init(pcDurableFileControl, szSubDirectory, "DAT", "Files.IDX", "_Files.IDX", iDataCacheSize, eWriteThrough, this);

	pcDurableFileControl->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::Kill(void)
{
	if (!mcData.IsFlushed())
	{
		return false;
	}
	if (!mbDescriptorsWritten)
	{
		return false;
	}

	mcData.Kill();

	mcDescriptorsFile.Kill();
	mcDescriptors.Kill();
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMap::InitIndices(CDurableFileController* pcDurableFileControl, bool bDirtyTesting)
{
	mbDescriptorsWritten = true;
	mcDescriptorsFile.Init(pcDurableFileControl, "Descriptors.IDX", "_Descriptors.IDX");

	if (mcDescriptorsFile.Exists())
	{
		mcDescriptors.Read(&mcDescriptorsFile);
		NullCachedDescriptors();
	}
	else
	{
		mcDescriptors.Init(true);
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
	bool					bExists;
	
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
bool CIndexedMap::DescriptorEvicted(OIndex oi, void* pvCache, uint32 uiDataSize)
{
	if (mpcIndexedDataEvictionCallback)
	{
		return mpcIndexedDataEvictionCallback->IndexEvicted(oi, pvCache, uiDataSize);
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction)
{
	CIndexedDataDescriptor* pcResult;

	pcResult = mcDescriptors.Get(oi);
	if (pcResult)
	{
		if (pcDescriptor)
		{
			memcpy_fast(pcDescriptor, pcResult, sizeof(CIndexedDataDescriptor));
		}
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction)
{
	CIndexedDataDescriptor* pcExistingDescriptor;
	bool					bUpdated;

	pcExistingDescriptor = mcDescriptors.Get(oi);
	if (!pcExistingDescriptor)
	{
		mbDescriptorsWritten = false;
		return mcDescriptors.Put(oi, pcDescriptor);
	}
	else
	{
		bUpdated = pcExistingDescriptor->Update(pcDescriptor);
		if (bUpdated)
		{
			mbDescriptorsWritten = false;
		}
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::UpdateDescriptorCache(OIndex oi, void* pvCache, uint32 uiDataSize)
{
	CIndexedDataDescriptor* pcDescriptor;

	pcDescriptor = mcDescriptors.Get(oi);
	if (pcDescriptor)
	{
		pcDescriptor->Cache(pvCache, uiDataSize);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::RemoveDescriptor(OIndex oi)
{
	mbDescriptorsWritten = false;
	return mcDescriptors.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::Flush(bool bClearCache)
{
	bool		bResult;

	bResult = mcData.Flush(bClearCache);
	if (!mbDescriptorsWritten)
	{
		bResult &= mcDescriptors.Write(&mcDescriptorsFile);
		mbDescriptorsWritten = true;
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
size CIndexedMap::NumIndicesCached(void)
{
	return mcDescriptors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::IsDirty(OIndex oi)
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
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedMap::TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return GetDescriptor(oi, pcDescriptor);
}

