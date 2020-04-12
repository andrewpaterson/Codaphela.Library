#include "BaseLib/Logger.h"
#include "IndexedCache.h"
#include "IndexedDataCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataCommon::Init(CIndexedEvictionCallback* pcEvictionCallback)
{
	mpcEvictionCallback = pcEvictionCallback;
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
		return gcLogger.Error2(__METHOD__, "Cannot Add index [", LongLongToString(oi), "].  It already exists.");
	}

	//This init clears the file index.  This means CompareDiskToMemory() will not try and read it to test for changes.
	return mcData.SetData(oi, NULL, pvData, uiDataSize, uiTimeStamp);
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
		return mcData.SetData(oi, &cDescriptor, pvData, cDescriptor.GetDataSize(), uiTimeStamp);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, "Cannot Set index [", LongLongToString(oi), "].  It does not exist.");
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
		return mcData.SetData(oi, &cDescriptor, pvData, uiDataSize, uiTimeStamp);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, "Cannot Set index [", LongLongToString(oi), "].  It does not exist.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Put(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL					bExists;
	CIndexedDataDescriptor	cDescriptor;

	bExists = GetDescriptor(oi, &cDescriptor);
	if (bExists)
	{
		return mcData.SetData(oi, &cDescriptor, pvData, uiDataSize, uiTimeStamp);
	}
	else
	{
		return mcData.SetData(oi, NULL, pvData, uiDataSize, uiTimeStamp);
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
	void*						pvCache;

	bResult = TRUE;
	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			pvCache = mcData.GetCachedData(psDesc);

			bResult &= UpdateDescriptorCache(psDesc->oi, NULL, 0);
			if (mpcEvictionCallback)
			{
				bResult &= mpcEvictionCallback->IndexEvicted(psDesc->oi, pvCache, psDesc->uiSize);
			}
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

	if (pvData)
	{
		if ((uiMaxSize >= uiDataSize) || (uiMaxSize == 0))
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
		else
		{
			return FALSE;
		}
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
int CIndexedDataCommon::NumDataCached(void)
 {
	return mcData.NumCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataCommon::NumDataCached(int iSize)
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
BOOL CIndexedDataCommon::IsCaching(void) { return mcData.IsCaching(); }

