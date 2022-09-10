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
bool CIndexedDataCommon::Add(OIndex oi, void* pvData, size_t uiDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor, true);
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
bool CIndexedDataCommon::Set(OIndex oi, void* pvData)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

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
bool CIndexedDataCommon::Set(OIndex oi, void* pvData, size_t uiDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

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
bool CIndexedDataCommon::Put(OIndex oi, void* pvData, size_t uiDataSize)
{
	bool					bExists;
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
	bool						bResult;
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
bool CIndexedDataCommon::Get(OIndex oi, void* pvData, size_t uiMaxSize)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot Get Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return false;
	}

	bResult = mcData.GetData(oi, &cDescriptor, pvData, uiMaxSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataCommon::Get(OIndex oi, size_t* puiDataSize, void* pvData, size_t uiMaxSize)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

	if (!IsValidIndex(oi))
	{
		SafeAssign(puiDataSize, 0);
		return gcLogger.Error2(__METHOD__, " Cannot Get Index [", IndexToString(oi), "].", NULL);
	}

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		SafeAssign(puiDataSize, 0);
		return false;
	}

	return GetData(oi, &cDescriptor, puiDataSize, pvData, uiMaxSize);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataCommon::GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, size_t* puiDataSize, void* pvData, size_t uiMaxSize)
{
	bool	bResult;
	size_t	uiDataSize;

	uiDataSize = pcDescriptor->GetDataSize();
	SafeAssign(puiDataSize, uiDataSize);

	if (pvData)
	{
		bResult = mcData.GetData(oi, pcDescriptor, pvData, uiMaxSize);
		if (bResult)
		{
			return true;
		}
		else
		{
			return false;
		}
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
bool CIndexedDataCommon::Contains(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

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
bool CIndexedDataCommon::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	bool					bResult;

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
		return true;
	}
	return false;
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
int CIndexedDataCommon::NumDataCached(size_t iSize)
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
int64 CIndexedDataCommon::NumDatas(size_t uiDataSize)
{
	return mcData.NumDatas(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataCommon::TestGetCachedObjectSize(OIndex oi) { return  mcData.TestGetCachedObjectSize(oi); }
bool CIndexedDataCommon::IsCaching(void) { return mcData.IsCaching();  }
bool CIndexedDataCommon::IsWriteThrough(void) { return meWriteThrough == IWT_Yes; }
bool CIndexedDataCommon::IsFlushed(void) { return mcData.IsFlushed(); }

