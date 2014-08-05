#include "PointerRemapper.h"
#include "IndexTreeNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	size_t tSize;

	muiFirstIndex = uiFirstIndex;
	muiLastIndex = uiLastIndex;
	muiDataSize = 0;
	mbNodesEmpty = FALSE;
	mpcParent = pcParent;

	tSize = (uiLastIndex - uiFirstIndex + 1) * sizeof(CIndexTreeNode*);
	memset(GetNodes(), 0, tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTreeNode* pcParent)
{
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
void CIndexTreeNode::Contain(unsigned char uiIndex)
{
	//Contain assumes that the memory this node resides in has already been sized large enough.

	unsigned char		uiOldNumIndexes;
	CIndexTreeNode**	pvDest;
	size_t				tSize;
	CIndexTreeNode**	apcChildren;

	apcChildren = GetNodes();

	if (mbNodesEmpty == TRUE)
	{
		mbNodesEmpty = FALSE;
		muiFirstIndex = uiIndex;
		muiLastIndex = uiIndex;
		apcChildren[0] = NULL;
		return;
	}

	uiOldNumIndexes = GetNumIndexes();
	apcChildren = GetNodes();

	if (uiIndex < muiFirstIndex)
	{
		tSize = (muiFirstIndex - uiIndex) * sizeof(CIndexTreeNode*);
		pvDest = (CIndexTreeNode**)RemapSinglePointer(apcChildren, tSize);
		memmove(pvDest, apcChildren, uiOldNumIndexes * sizeof(CIndexTreeNode*));
		memset(apcChildren, 0, tSize);
		muiFirstIndex = uiIndex;
	}
	else if (uiIndex > muiLastIndex)
	{
		tSize = (uiIndex - muiLastIndex) * sizeof(CIndexTreeNode*);
		pvDest = (CIndexTreeNode**)RemapSinglePointer(apcChildren, uiOldNumIndexes * sizeof(CIndexTreeNode*));
		memset(pvDest, 0, tSize);
		muiLastIndex = uiIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::Uncontain(unsigned char uiIndex)
{
	unsigned char		uiNextFirstIndex;
	unsigned char		uiPrevLastIndex;
	int					iNewNumIndexes;
	size_t				tSize;
	void*				pvSource;
	CIndexTreeNode**	apcChildren;

	if ((uiIndex != muiFirstIndex) && (uiIndex != muiLastIndex))
	{
		return FALSE;
	}
	else if (muiFirstIndex == muiLastIndex)
	{
		mbNodesEmpty = TRUE;
		muiFirstIndex = 0;
		muiLastIndex = 0;
		return TRUE;
	}

	apcChildren = GetNodes();

	if (uiIndex == muiFirstIndex)
	{
		uiNextFirstIndex = FindNextFirstIndex();
		iNewNumIndexes = GetNumIndexes(uiNextFirstIndex, muiLastIndex);
		tSize = (uiNextFirstIndex - muiFirstIndex) * sizeof(CIndexTreeNode*);
		pvSource = (CIndexTreeNode**)RemapSinglePointer(apcChildren, tSize);
		memmove(apcChildren, pvSource, iNewNumIndexes * sizeof(CIndexTreeNode*));
		muiFirstIndex = uiNextFirstIndex;
	}
	else if (uiIndex == muiLastIndex)
	{
		uiPrevLastIndex = FindPrevLastIndex();
		muiLastIndex = uiPrevLastIndex;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeNode::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return GetNode(uiIndex - muiFirstIndex);
	}
	else
	{
		return NULL;
	}
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
		return RemapSinglePointer(this, sizeof(CIndexTreeNode));
	}
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
void CIndexTreeNode::Set(unsigned char uiIndex, CIndexTreeNode* pcNode)
{
	CIndexTreeNode**	apcChildren;

	if (ContainsIndex(uiIndex))
	{
		apcChildren = GetNodes();
		apcChildren[uiIndex - muiFirstIndex] = pcNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::Clear(unsigned char uiIndex)
{
	CIndexTreeNode**	apcChildren;

	if (ContainsIndex(uiIndex))
	{
		apcChildren = GetNodes();
		apcChildren[uiIndex - muiFirstIndex] = NULL;

		return Uncontain(uiIndex);
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
unsigned char CIndexTreeNode::FindNextFirstIndex(void)
{
	int					i;
	CIndexTreeNode*		pcChild;
	CIndexTreeNode**	apcChildren;

	apcChildren = GetNodes();
	for (i = 1; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return muiFirstIndex + i;
		}
	}
	return muiLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::FindPrevLastIndex(void)
{
	int					i;
	CIndexTreeNode*		pcChild;
	CIndexTreeNode**	apcChildren;

	apcChildren = GetNodes();
	for (i = (int)(muiLastIndex - muiFirstIndex)-1; i >= 0; i--)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return muiFirstIndex + i;
		}
	}
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::FindIndex(CIndexTreeNode* pcChild)
{
	int					i;
	CIndexTreeNode*		pcChildTest;
	CIndexTreeNode**	apcChildren;

	apcChildren = GetNodes();
	for (i = 0; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChildTest = apcChildren[i];
		if (pcChild == pcChildTest)
		{
			return i + muiFirstIndex;
		}
	}

	//This should never be hit.
	return 0;
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
		memcpy(GetObjectPtr(), pvObject, uiSize);
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
BOOL CIndexTreeNode::IsEmpty(void)
{
	int					i;
	CIndexTreeNode*		pcChild;
	CIndexTreeNode**	apcChildren;

	if (muiDataSize != 0)
	{
		return FALSE;
	}

	if (mbNodesEmpty == TRUE)
	{
		return TRUE;
	}

	apcChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return FALSE;
		}
	}
	return TRUE;
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
	return mbNodesEmpty == FALSE;
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

	return (uiLastIndex - uiFirstIndex + 1);   //+1 because uiLast is inclusive.  -1 because CIndexTreeNode already includes 1.	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForIndex(unsigned char uiIndex)
{
	size_t	tSize;
	int		iNewIndices;

	iNewIndices = GetAdditionalIndexes(uiIndex);
	tSize = sizeof(CIndexTreeNode) + muiDataSize + iNewIndices * sizeof(CIndexTreeNode*);

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForEmpty(void)
{
	return sizeof(CIndexTreeNode) + muiDataSize + sizeof(CIndexTreeNode*);
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
	tSize = sizeof(CIndexTreeNode) + uiDataSize + iExistingIndices * sizeof(CIndexTreeNode*);

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
	tSize = sizeof(CIndexTreeNode) + muiDataSize + iExistingIndices * sizeof(CIndexTreeNode*);

	return tSize;
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
	return uiLastIndex - uiFirstIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeNode::GetNode(int i)
{
	return GetNodes()[i];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode** CIndexTreeNode::GetNodes(void)
{
	return (CIndexTreeNode**)RemapSinglePointer(this, sizeof(CIndexTreeNode) + muiDataSize);
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
void CIndexTreeNode::RemapChildNodes(CIndexTreeNode* pcOldNode, CIndexTreeNode* pcNewNode)
{
	int					i;
	int					iNumNodes;
	CIndexTreeNode**	apcChildren;

	iNumNodes = GetNumIndexes();
	apcChildren = GetNodes();

	for (i = 0; i < iNumNodes; i++)
	{
		if (apcChildren[i] == pcOldNode)
		{
			apcChildren[i] = pcNewNode;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetChildsParent(void)
{
	int					i;
	int					iNumNodes;
	CIndexTreeNode*		pcChild;
	CIndexTreeNode**	apcChildren;

	iNumNodes = GetNumIndexes();
	apcChildren = GetNodes();

	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild)
		{
			pcChild->mpcParent = this;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SizeObject(unsigned char uiSize)
{
	size_t				tSize;
	CIndexTreeNode**	apcChildren;
	int					iDiff;
	CIndexTreeNode**	apcMovedChildren;

	if (!mbNodesEmpty)
	{
		apcChildren = GetNodes();
		tSize = (muiLastIndex - muiFirstIndex + 1) * sizeof(CIndexTreeNode*);

		iDiff = (int)uiSize - (int)muiDataSize;

		apcMovedChildren = (CIndexTreeNode**)RemapSinglePointer(apcChildren, iDiff);
		memmove(apcMovedChildren, apcChildren, tSize);
	}

	muiDataSize = uiSize;
}

