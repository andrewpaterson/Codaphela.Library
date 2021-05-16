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
	mbStarted = FALSE;
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
void* CIndexMapIterator::Iterate(size_t* puiDataSize, void* pvDestKey, size_t* puiKeySize, size_t uiMaxKeySize)
{
	void*	pvData;
	BOOL	bResult;

	if (mbStarted)
	{
		bResult = mpcIndex->StartIteration(&msIter, &pvData, puiDataSize, pvDestKey, puiKeySize, uiMaxKeySize);
	}
	else
	{
		mbStarted = TRUE;
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

