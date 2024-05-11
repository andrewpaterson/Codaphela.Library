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
void* CMapMapIterator::Iterate(uint32* puiDataSize, void* pvDestKey, uint32* puiKeySize, uint32 uiMaxKeySize)
{
	void*	pvKey;
	uint32	iKeySize;
	void*	pvData;
	uint32	iDataSize;
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

