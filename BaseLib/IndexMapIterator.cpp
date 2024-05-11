#include "PointerFunctions.h"
#include "MapHelper.h"
#include "IndexMapIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMapIterator::Init(CIndexBlock* pcIndex)
{
	mpcIndex = pcIndex;
	mbStarted = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMapIterator::Kill(void)
{
	mpcIndex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexMapIterator::Iterate(size* puiDataSize, void* pvDestKey, size* puiKeySize, size uiMaxKeySize)
{
	void*	pvData;
	bool	bResult;

	if (mbStarted)
	{
		bResult = mpcIndex->StartIteration(&msIter, &pvData, puiDataSize, pvDestKey, puiKeySize, uiMaxKeySize);
	}
	else
	{
		mbStarted = true;
		bResult = mpcIndex->Iterate(&msIter, &pvData, puiDataSize, pvDestKey, puiKeySize, uiMaxKeySize);
	}

	if (bResult)
	{
		return pvData;
	}
	else
	{
		return NULL;
	}
}

