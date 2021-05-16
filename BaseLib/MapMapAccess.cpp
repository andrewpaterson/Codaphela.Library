#include "MapBlock.h"
#include "MapMapIterator.h"
#include "MapMapAccess.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapMapAccess::Init(CMapBlock* pcMap)
{
	mpcMap = pcMap;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapMapAccess::Kill(void)
{
	mpcMap = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CMapMapAccess::NumElements(void)
{
	return mpcMap->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapMapAccess::Dump(void)
{
	mpcMap->Dump();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMapIterator* CMapMapAccess::CreateIterator(void)
{
	CMapMapIterator* pcIter;

	pcIter = NewMalloc<CMapMapIterator>();
	pcIter->Init(mpcMap);

	return pcIter;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapMapAccess::FreeIterator(CMapIterator* pcIter)
{
	SafeKill(pcIter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapMapAccess::Put(void* pvKey, int iKeySize, void* pvData, size_t uiDataSize)
{
	return mpcMap->Put(pvKey, iKeySize, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMapMapAccess::Get(void* pvKey, int iKeySize, size_t* puiDataSize)
{
	BOOL	bResult;
	void*	pvData;

	bResult = mpcMap->Get(pvKey, iKeySize, &pvData, (int*)puiDataSize);
	if (bResult)
	{
		return pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size_t CMapMapAccess::DataSize(void* pvKey, int iKeySize)
{
	return mpcMap->DataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapMapAccess::Remove(void* pvKey, int iKeySize)
{
	return mpcMap->Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapMapAccess::Has(void* pvKey, int iKeySize)
{
	return mpcMap->HasKey(pvKey, iKeySize);
}

