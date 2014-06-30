#include "PointerRemapper.h"
#include "IndexTreeNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	size_t tSize;

	mpvObject = NULL;
	muiFirstIndex = uiFirstIndex;
	muiLastIndex = uiLastIndex;
	muiDataSize = 0;
	mbNodesEmpty = FALSE;
	mpcParent = pcParent;

	if (muiFirstIndex == muiLastIndex)
	{
		mapcChildren[0] = NULL;
	}
	else
	{
		tSize = (uiLastIndex - uiFirstIndex + 1) * sizeof(CIndexTreeNode*);
		memset(mapcChildren, 0, tSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTreeNode* pcParent)
{
	mpvObject = NULL;
	mpcParent = pcParent;
	muiFirstIndex = 0;
	muiLastIndex = 0;
	muiDataSize = 0;
	mbNodesEmpty = TRUE;
	mapcChildren[0] = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Contain(unsigned char uiIndex)
{
	unsigned char	uiOldNumIndexes;
	void*			pvDest;
	size_t			tSize;

	if (mbNodesEmpty == TRUE)
	{
		mbNodesEmpty = FALSE;
		muiFirstIndex = uiIndex;
		muiLastIndex = uiIndex;
		return;
	}

	uiOldNumIndexes = GetNumIndexes();

	if (uiIndex < muiFirstIndex)
	{
		tSize = (muiFirstIndex - uiIndex) * sizeof(CIndexTreeNode*);
		pvDest = RemapSinglePointer(mapcChildren, tSize);
		memcpy(pvDest, mapcChildren, uiOldNumIndexes * sizeof(CIndexTreeNode*));
		memset(mapcChildren, 0, tSize);
		muiFirstIndex = uiIndex;
	}
	else if (uiIndex > muiLastIndex)
	{
		tSize = (uiIndex - muiLastIndex) * sizeof(CIndexTreeNode*);
		pvDest = RemapSinglePointer(mapcChildren, uiOldNumIndexes * sizeof(CIndexTreeNode*));
		memset(pvDest, 0, tSize);
		muiLastIndex = uiIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeNode::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return mapcChildren[uiIndex - muiFirstIndex];
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
void* CIndexTreeNode::GetObject(void)
{
	return mpvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Set(unsigned char uiIndex, CIndexTreeNode* pcNode)
{
	if (ContainsIndex(uiIndex))
	{
		mapcChildren[uiIndex - muiFirstIndex] = pcNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Clear(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		mapcChildren[uiIndex - muiFirstIndex] = NULL;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::SetObject(void* pvObject)
{
	if (mpvObject == NULL)
	{
		mpvObject = pvObject;
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
	mpvObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsEmpty(void)
{
	int					i;
	CIndexTreeNode*		pcChild;

	if (mpvObject != NULL)
	{
		return FALSE;
	}

	if (mbNodesEmpty == TRUE)
	{
		return TRUE;
	}

	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = mapcChildren[i];
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

	return ((uiLastIndex - uiFirstIndex + 1) - 1);   //+1 because uiLast is inclusive.  -1 because CIndexTreeNode already includes 1.	
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::GetNumIndexes(void)
{
	return muiLastIndex - muiFirstIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeNode::GetNode(int i)
{
	return mapcChildren[i];
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
	int		i;
	int		iNumNodes;

	iNumNodes = GetNumIndexes();

	for (i = 0; i < iNumNodes; i++)
	{
		if (mapcChildren[i] == pcOldNode)
		{
			mapcChildren[i] = pcNewNode;
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

	iNumNodes = GetNumIndexes();

	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = mapcChildren[i];
		if (pcChild)
		{
			pcChild->mpcParent = this;
		}
	}
}

