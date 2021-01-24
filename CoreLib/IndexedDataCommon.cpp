#include "BaseLib/Logger.h"
#include "IndexedCache.h"
#include "IndexedDataCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataCommon::Init(CIndexedDataEvictionCallback* pcIndexedDataEvictionCallback)
{
	mpcIndexedDataEvictionCallback = pcIndexedDataEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor, TRUE);
	if (bResult)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].  It already exists.", NULL);
	}

	//This init clears the file index.  This means CompareDiskToMemory() will not try and read it to test for changes.
	return mcData.SetData(oi, NULL, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Set(OIndex oi, void* pvData)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return mcData.SetData(oi, &cDescriptor, pvData, cDescriptor.GetDataSize());
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "].  It does not exist.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return mcData.SetData(oi, &cDescriptor, pvData, uiDataSize);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set Index [", IndexToString(oi), "].  It does not exist.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Put(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	BOOL					bExists;
	CIndexedDataDescriptor	cDescriptor;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put Index [", IndexToString(oi), "].", NULL);
	}

	bExists = GetDescriptor(oi, &cDescriptor);
	if (bExists)
	{
		return mcData.SetData(oi, &cDescriptor, pvData, uiDataSize);
	}
	else
	{
		return mcData.SetData(oi, NULL, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::Size(OIndex oi)
{
	BOOL						bResult;
	CIndexedDataDescriptor		cDescriptor;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot get Size of Index [", IndexToString(oi), "].", NULL);
	}

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
BOOL CIndexedDataCommon::Get(OIndex oi, void* pvData, unsigned int uiMaxSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Get Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mcData.GetData(oi, &cDescriptor, pvData, uiMaxSize);
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

	if (!IsValidIndex(oi))
	{
		SafeAssign(puiDataSize, 0);
		return gcLogger.Error2(__METHOD__, " Cannot Get Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		SafeAssign(puiDataSize, 0);
		return FALSE;
	}

	uiDataSize = cDescriptor.GetDataSize();
	SafeAssign(puiDataSize, (int)uiDataSize);

	if (pvData)
	{
		bResult = mcData.GetData(oi, &cDescriptor, pvData, uiMaxSize);
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
int64 CIndexedDataCommon::NumElements(void)
{
	return NumIndices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataCommon::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Remove Index [", IndexToString(oi), "].", NULL);
	}

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
int64 CIndexedDataCommon::NumDatas()
{
	return mcData.NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDataCommon::NumDatas(unsigned int uiDataSize)
{
	return mcData.NumDatas(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::TestGetCachedObjectSize(OIndex oi) { return  mcData.TestGetCachedObjectSize(oi); }
BOOL CIndexedDataCommon::IsCaching(void) { return mcData.IsCaching();  }
BOOL CIndexedDataCommon::IsWriteThrough(void) { return meWriteThrough == IWT_Yes; }
BOOL CIndexedDataCommon::IsFlushed(void) { return mcData.IsFlushed(); }

