#include "IndexedCache.h"
#include "IndexedFilesEvictedDescriptorList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iDataCacheSize, BOOL bWriteThrough)
{
	mcEvicted.Init();

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
	mcEvicted.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFilesEvictedDescriptorList::NumEvicted(void)
{
	return mcEvicted.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedFilesEvictedDescriptorList::GetEvicted(int iIndex)
{
	return mcEvicted.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::ClearEvicted(void)
{
	mcEvicted.ReInit();
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
	CIndexedDataDescriptor*	pcDesc;
	BOOL					bExists;
	
	bExists = mcDescriptors.StartIteration(&sIter, (void**)&poi, (void**)&pcDesc);
	while (bExists)
	{
		pcDesc->Cache(NULL);
		bExists = mcDescriptors.Iterate(&sIter, (void**)&poi, (void**)&pcDesc);
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
	int							iNumElements;
	void*						pvData;

	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			pvData = RemapSinglePointer(psDesc, sizeof(SIndexedCacheDescriptor));
			mcEvicted.Add(pvData, psDesc->iDataSize);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
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
BOOL CIndexedFilesEvictedDescriptorList::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
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
BOOL CIndexedFilesEvictedDescriptorList::UpdateDescriptorCache(OIndex oi, void* pvCache)
{
	CIndexedDataDescriptor* pcDescriptor;

	pcDescriptor = mcDescriptors.Get(oi);
	if (pcDescriptor)
	{
		pcDescriptor->Cache(pvCache);
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
BOOL CIndexedFilesEvictedDescriptorList::TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return GetDescriptor(oi, pcDescriptor);
}

