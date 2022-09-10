#include "PointerFunctions.h"
#include "MapHelper.h"
#include "MapMapIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapMapIterator::Init(CMapBlock* pcMapBlock)
{
	mpcMap = pcMapBlock;
	mbStarted = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapMapIterator::Kill(void)
{
	mpcMap = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapMapIterator::Iterate(size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	void*	pvKey;
	int		iKeySize;
	void*	pvData;
	int		iDataSize;
	bool	bResult;

	if (mbStarted)
	{
		bResult = mpcMap->Iterate(&msIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	else
	{
		mbStarted = true;
		bResult = mpcMap->StartIteration(&msIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}

	if (bResult)
	{
		if (pvDestKey)
		{
			memcpy(pvDestKey, pvKey, MinDataSize(iKeySize, uiMaxKeySize));
		}
		SafeAssign(puiDataSize, iDataSize);
		SafeAssign(puiKeySize, iKeySize);
		return pvData;
	}
	else
	{
		return NULL;
	}
}

