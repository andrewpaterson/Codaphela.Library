#include "IndexTreeMemoryIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryIterator::Init(CIndexTreeMemory* pcIndexTree)
{
	CIndexTreeIterator::Init(INDEX_TREE_MEMORY_ITERATOR_TYPE);
	mpcTree = pcIndexTree;
	sIter.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryIterator::Kill(void)
{
	sIter.Kill();
	CIndexTreeIterator::Kill();
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeMemoryIterator::Iterate(void)
{
	bool	bHasData;

	if (mbStarted)
	{
		bHasData = mpcTree->Iterate(&sIter, NULL, &miKeySize, 0, &macData, &miDataSize, MAX_DATA_SIZE);
	}
	else
	{
		bHasData = mpcTree->StartIteration(&sIter, NULL, &miKeySize, 0, &macData, &miDataSize, MAX_DATA_SIZE);
		mbStarted = true;
	}

	if (bHasData)
	{
		SetKey(sIter.acKey.GetData(), sIter.iKeyLength);
	}
	else
	{
		Clear();
	}

	return bHasData;
}

