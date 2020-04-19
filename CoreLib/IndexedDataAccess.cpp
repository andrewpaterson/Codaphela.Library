#include "IndexedDataAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataAccess::Init(CIndexedData* pcIndexData)
{
	CIndexTreeAccess::Init();
	mpcIndexData = pcIndexData;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataAccess::Kill(void)
{
	mpcIndexData = NULL;
	return CIndexTreeAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataAccess::Flush(void)
{
	return mpcIndexData->Flush(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataAccess::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Put(oi, pvData, iDataSize);
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
BOOL CIndexedDataAccess::Get(void* pvKey, int iKeySize, void* pvData, int* piDataSize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Get(oi, (unsigned int*)piDataSize, pvData, 0);
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
int CIndexedDataAccess::DataSize(void* pvKey, int iKeySize)
{
	OIndex			oi;
	unsigned int	uiDataSize;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		if (mpcIndexData->Get(oi, &uiDataSize, NULL, 0))
		{
			return uiDataSize;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataAccess::Remove(void* pvKey, int iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Remove(oi);
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
BOOL CIndexedDataAccess::Has(void* pvKey, int iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Contains(oi);
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
BOOL CIndexedDataAccess::Flush(void* pvKey, int iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->FlushKey(oi);
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
BOOL CIndexedDataAccess::Evict(void* pvKey, int iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->EvictKey(oi);
	}
	else
	{
		return FALSE;
	}
}

