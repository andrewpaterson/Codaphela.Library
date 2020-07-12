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
		bHasData = mpcTree->Iterate(&sIter, &macKey, &miKeySize, &macData, &miDataSize);
	}
	else
	{
		bHasData = mpcTree->StartIteration(&sIter, &macKey, &miKeySize, &macData, &miDataSize);
		mbStarted = TRUE;
	}

	if (!bHasData)
	{
		Clear();
	}

	return bHasData;
}

