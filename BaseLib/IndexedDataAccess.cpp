#include "IndexedDataAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataAccess::Init(CIndexedData* pcIndexData)
{
	CIndexAccess::Init();
	mpcIndexData = pcIndexData;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataAccess::Kill(void)
{
	mpcIndexData = NULL;
	return CIndexAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataAccess::Dump(void)
{
	mpcIndexData->DumpIndex();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataAccess::Flush(void)
{
	return mpcIndexData->Flush(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDataAccess::NumElements(void)
{
	return mpcIndexData->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataAccess::ValidateIndex(void)
{
	return mpcIndexData->ValidateIndex();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataAccess::Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Put(oi, pvData, iDataSize);
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
bool CIndexedDataAccess::Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Get(oi, (uint32*)piDataSize, pvData, uiMaxDataSize);
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
size CIndexedDataAccess::DataSize(uint8* pvKey, size iKeySize)
{
	OIndex			oi;
	uint32	uiDataSize;

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
bool CIndexedDataAccess::Remove(uint8* pvKey, size iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Remove(oi);
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
bool CIndexedDataAccess::Has(uint8* pvKey, size iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->Contains(oi);
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
bool CIndexedDataAccess::Flush(uint8* pvKey, size iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->FlushKey(oi);
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
bool CIndexedDataAccess::Evict(uint8* pvKey, size iKeySize)
{
	OIndex	oi;

	if (iKeySize == sizeof(OIndex))
	{
		oi = *((OIndex*)pvKey);
		return mpcIndexData->EvictKey(oi);
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
CIndexTreeIterator* CIndexedDataAccess::CreateIterator(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataAccess::FreeIterator(CIndexTreeIterator* pcIter)
{
}

