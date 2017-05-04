#include "IndexTree.h"
#include "IndexTreeNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize, int iClearValue)
{
	size_t tSize;

	mpcIndexTree = pcIndexTree;
	muiFirstIndex = uiFirstIndex;
	muiLastIndex = uiLastIndex;
	muiDataSize = uiDataSize;
	mbNodesEmpty = FALSE;
	mpcParent = pcParent;

	tSize = (uiLastIndex - uiFirstIndex + 1) * SizeofNodePtr();
	memset(GetNodesMemory(), iClearValue, tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent)
{
	mpcIndexTree = pcIndexTree;
	mpcParent = pcParent;
	muiFirstIndex = 0;
	muiLastIndex = 0;
	muiDataSize = 0;
	mbNodesEmpty = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetObjectSize(void)
{
	return muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeNode::GetObjectPtr(void)
{
	if (muiDataSize == 0)
	{
		return NULL;
	}
	else
	{
		return RemapSinglePointer(this, SizeofNode());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::SizeofNode(void)
{
	return mpcIndexTree->SizeofNode();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::SizeofNodePtr(void)
{
	return mpcIndexTree->SizeofNodePtr();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeNode::GetNodesMemory(void)
{
	return RemapSinglePointer(this, SizeofNode() + muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeNode::GetParent(void)
{
	return mpcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForIndex(unsigned char uiIndex)
{
	size_t	tSize;
	int		iRequiredIndices;

	if (HasNodes())
	{
		iRequiredIndices = GetAdditionalIndexes(uiIndex);
//		mpcIndexTree->Calc
		tSize = SizeofNode() + muiDataSize + iRequiredIndices * SizeofNodePtr();

		return tSize;
	}
	else
	{
		return CalculateRequiredNodeSizeForEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForEmpty(void)
{
	return mpcIndexTree->CalculateNodeSize(1, muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForData(unsigned char uiDataSize)
{
	size_t	tSize;
	int		iExistingIndices;

	iExistingIndices = GetNumIndexes();
	tSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, uiDataSize);

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForCurrent(void)
{
	size_t	tSize;
	int		iExistingIndices;

	iExistingIndices = GetNumIndexes();
	tSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, muiDataSize);

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearOnlyNode(unsigned char uiIndex, int iClearValue)
{
	size_t	tSize;
	void*	pvNodes;

	pvNodes = GetNodesMemory();
	muiFirstIndex = uiIndex;
	muiLastIndex = uiIndex;
	tSize = SizeofNodePtr();
	memset(pvNodes, iClearValue, tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::MoveNodesLeft(unsigned char uiNextFirstIndex)
{
	size_t	tSize;
	void*	pvSource;
	void*	pvNodes;
	int		iNewNumIndexes;

	pvNodes = GetNodesMemory();

	iNewNumIndexes = GetNumIndexes(uiNextFirstIndex, muiLastIndex);
	tSize = (uiNextFirstIndex - muiFirstIndex) * SizeofNodePtr();
	pvSource = (CIndexTreeNode**)RemapSinglePointer(pvNodes, tSize);
	memmove(pvNodes, pvSource, iNewNumIndexes * SizeofNodePtr());
	muiFirstIndex = uiNextFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::MoveNodesRight(unsigned char uiNewFirstIndex, int iClearValue)
{
	size_t		tSize;
	void*		pvNodes;
	void*		pvDest;
	int			uiOldNumIndexes;

	pvNodes = GetNodesMemory();
	uiOldNumIndexes = GetNumIndexes();

	tSize = (muiFirstIndex - uiNewFirstIndex) * SizeofNodePtr();
	pvDest = (CIndexTreeNode**)RemapSinglePointer(pvNodes, tSize);
	memmove(pvDest, pvNodes, uiOldNumIndexes * SizeofNodePtr());
	memset(pvNodes, iClearValue, tSize);
	muiFirstIndex = uiNewFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearLastNodes(unsigned char uiNewLastIndex, int iClearValue)
{
	int			uiOldNumIndexes;
	void*		pvDest;
	size_t		tSize;
	void*		pvNodes;

	pvNodes = GetNodesMemory();
	uiOldNumIndexes = GetNumIndexes();

	tSize = (uiNewLastIndex - muiLastIndex) * SizeofNodePtr();
	pvDest = (CIndexTreeNode**)RemapSinglePointer(pvNodes, uiOldNumIndexes * SizeofNodePtr());
	memset(pvDest, iClearValue, tSize);
	muiLastIndex = uiNewLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::GetNumIndexes(void)
{
	if (mbNodesEmpty)
	{
		return 0;
	}
	return GetNumIndexes(muiFirstIndex, muiLastIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::GetNumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	return (uiLastIndex - uiFirstIndex) + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::GetAdditionalIndexes(unsigned char uiIndex)
{
	unsigned char uiFirstIndex;
	unsigned char uiLastIndex;

	if (uiIndex < muiFirstIndex)
	{
		uiFirstIndex = uiIndex;
		uiLastIndex = muiLastIndex;
	}
	else if (uiIndex > muiLastIndex)
	{
		uiFirstIndex = muiFirstIndex;
		uiLastIndex = uiIndex;
	}
	else
	{
		uiFirstIndex = muiFirstIndex;
		uiLastIndex = muiLastIndex;
	}

	return (uiLastIndex - uiFirstIndex + 1);   //+1 because uiLast is inclusive.  -1 because CIndexTreeNodeMemory already includes 1.	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsEmpty(void)
{
	if (muiDataSize != 0)
	{
		return FALSE;
	}

	return mbNodesEmpty;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetFirstIndex(void)
{
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetLastIndex(void)
{
	return muiLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::HasNodes(void)
{
	return !mbNodesEmpty;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::HasObject(void)
{
	return muiDataSize != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::NumNodes(void)
{
	return (unsigned char)GetNumIndexes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::ContainsIndex(unsigned char uiIndex)
{
	if (mbNodesEmpty)
	{
		return FALSE;
	}

	if ((uiIndex >= muiFirstIndex) && (uiIndex <= muiLastIndex))
	{
		return TRUE;
	}	
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::SetObject(void* pvObject, unsigned char uiSize)
{
	if (muiDataSize == 0)
	{
		SizeObject(uiSize);
		if (pvObject)
		{
			memcpy_fast(GetObjectPtr(), pvObject, uiSize);
		}
		return TRUE;
	}
	else if (muiDataSize == muiDataSize)
	{
		if (pvObject)
		{
			memcpy_fast(GetObjectPtr(), pvObject, uiSize);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearObject(void)
{
	SizeObject(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SizeObject(unsigned char uiSize)
{
	size_t				tSize;
	void*				apcChildren;
	int					iDiff;
	void*				apcMovedChildren;

	if (!mbNodesEmpty)
	{
		apcChildren = GetNodesMemory();
		tSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();

		iDiff = (int)uiSize - (int)muiDataSize;

		apcMovedChildren = RemapSinglePointer(apcChildren, iDiff);
		memmove(apcMovedChildren, apcChildren, tSize);
	}

	muiDataSize = uiSize;
}

