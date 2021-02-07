#include "IndexTreeFileIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileIterator::Init(CIndexTreeFile* pcIndexTree)
{
	CIndexTreeIterator::Init(INDEX_TREE_FILE_ITERATOR_TYPE);
	mpcTree = pcIndexTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileIterator::Kill(void)
{
	CIndexTreeIterator::Kill();
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileIterator::Iterate(void)
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

