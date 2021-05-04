#include "IndexTreeMemoryIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryIterator::Init(CIndexTreeMemory* pcIndexTree)
{
	CIndexTreeIterator::Init(INDEX_TREE_MEMORY_ITERATOR_TYPE);
	mpcTree = pcIndexTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryIterator::Kill(void)
{
	CIndexTreeIterator::Kill();
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemoryIterator::Iterate(void)
{
	BOOL	bHasData;

	if (mbStarted)
	{
		bHasData = mpcTree->Iterate(&sIter, &macKey, &miKeySize, MAX_KEY_SIZE, &macData, &miDataSize, MAX_DATA_SIZE);
	}
	else
	{
		bHasData = mpcTree->StartIteration(&sIter, &macKey, &miKeySize, MAX_KEY_SIZE, &macData, &miDataSize, MAX_DATA_SIZE);
		mbStarted = TRUE;
	}

	if (!bHasData)
	{
		Clear();
	}

	return bHasData;
}

