#include "IndexedCache.h"
#include "IndexedFilesEvictedDescriptorList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming("");
	cConfig.mszRewriteDirectory = "";
	cConfig.SetObjectCacheSize(iCacheSize);

	mcData.Init(pcDurableFileControl, szDataExtension, szDescricptorName, szDescricptorRewrite, iCacheSize, bWriteThrough, this);

	mcEvicted.Init();

	CIndexedDataCommon::Init(&cConfig, pcDurableFileControl, this);
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
	mbDescriptorsWritten = FALSE;
	return mcDescriptors.Put(oi, pcDescriptor);
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
BOOL CIndexedFilesEvictedDescriptorList::TestGetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return GetDescriptor(oi, pcDescriptor);
}

