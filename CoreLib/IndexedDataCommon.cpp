#include "IndexedCache.h"
#include "IndexedDataCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataCommon::Init(CIndexedConfig* pcConfig, CDurableFileController* pcDurableFileControl, CIndexedFilesEvictionCallback* pcEvictionCallback)
{
	mbWriteThrough = pcConfig->mbWriteThrough;

	pcDurableFileControl->Begin();

	InitIndices(pcDurableFileControl, pcConfig->mbDirtyTesting);
	mcData.Init(pcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", pcConfig->miObjectsCacheSize, pcConfig->mbWriteThrough, pcEvictionCallback);

	pcDurableFileControl->End();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		//Can't add an oi that already exists.
		return FALSE;
	}

	//This init clears the file index.  This means CompareDiskToMemory() will not try and read it to test for changes.
	cDescriptor.Init(uiDataSize);

	bResult = mcData.SetData(oi, &cDescriptor, pvData, uiTimeStamp);
	bResult &= SetDescriptor(oi, &cDescriptor);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Set(OIndex oi, void* pvData, unsigned int uiTimeStamp)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return SetData(oi, &cDescriptor, pvData, uiTimeStamp);
	}
	else
	{
		//Can't set if the oi doesn't exist.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp)
{
	BOOL	bResult;

	if (pcDescriptor->IsCached())
	{
		bResult = mcData.SetData(pcDescriptor, pvData);
		bResult &= SetDescriptor(oi, pcDescriptor);
		return bResult;
	}
	else
	{
		bResult = mcData.SetData(oi, pcDescriptor, pvData, uiTimeStamp);
		bResult &= SetDescriptor(oi, pcDescriptor);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return SetData(oi, &cDescriptor, pvData, uiDataSize, uiTimeStamp);
	}
	else
	{
		//Can't set if the oi doesn't exist.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL	bResult;

	if (pcDescriptor->GetDataSize() == uiDataSize)
	{
		return SetData(oi, pcDescriptor, pvData, uiTimeStamp);
	}
	else
	{
		mcData.InvalidateData(pcDescriptor);
		pcDescriptor->Init(uiDataSize);

		bResult = mcData.SetData(oi, pcDescriptor, pvData, uiTimeStamp);
		bResult &= SetDescriptor(oi, pcDescriptor);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL					bResult;
	CIndexedDataDescriptor	cDescriptor;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return Set(oi, pvData, uiDataSize, uiTimeStamp);
	}
	else
	{
		return Add(oi, pvData, uiDataSize, uiTimeStamp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	BOOL						bResult;
	int							iNumElements;

	bResult = TRUE;
	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			bResult &= UpdateDescriptorCache(psDesc->oi, NULL);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::Size(OIndex oi)
{
	BOOL						bResult;
	CIndexedDataDescriptor		cDescriptor;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return cDescriptor.GetDataSize();
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Get(OIndex oi, void* pvData)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mcData.GetData(oi, &cDescriptor, pvData);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;
	unsigned int			uiDataSize;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	uiDataSize = cDescriptor.GetDataSize();
	SafeAssign(puiDataSize, (int)uiDataSize);

	if (pvData && uiMaxSize >= uiDataSize)
	{
		bResult = mcData.GetData(oi, &cDescriptor, pvData);
		if (bResult)
		{
			return TRUE;
		}
		else
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
BOOL CIndexedDataCommon::Contains(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		if (cDescriptor.IsCached())
		{
			mcData.InvalidateData(&cDescriptor);
		}
		RemoveDescriptor(oi);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::Flags(OIndex oi)
{
	CIndexedDataDescriptor		cDescriptor;
	BOOL						bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return cDescriptor.GetUserFlags();
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataCommon::NumCached(void)
{
	return mcData.NumCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataCommon::NumCached(int iSize)
{
	return mcData.NumCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataCommon::NumFiles(void)
{
	return mcData.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDataCommon::NumData(unsigned int uiDataSize)
{
	return mcData.NumData(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::TestGetCachedObjectSize(OIndex oi) { return  mcData.TestGetCachedObjectSize(oi); }
int CIndexedDataCommon::TestNumIgnoredCacheElements(void) { return mcData.TestNumIgnoredCacheElements(); }
BOOL CIndexedDataCommon::IsCaching(void) { return mcData.IsCaching(); }

