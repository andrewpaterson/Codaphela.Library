#include "Logger.h"
#include "LogString.h"
#include "IndexTreeNodeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeNodeMemory::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return (CIndexTreeNodeMemory*)GetNode(uiIndex - muiFirstIndex);
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
void CIndexTreeNodeMemory::Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode)
{
	CIndexTreeNodeMemory**	apcChildren;

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
void CIndexTreeNodeMemory::Clear(unsigned char uiIndex)
{
	CIndexTreeNodeMemory**	apcChildren;

	if (ContainsIndex(uiIndex))
	{
		apcChildren = GetNodes();
		apcChildren[uiIndex - muiFirstIndex] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::ClearAndUncontain(unsigned char uiIndex)
{
	CIndexTreeNodeMemory**	apcChildren;

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
CIndexTreeNodeMemory* CIndexTreeNodeMemory::GetNode(int i)
{
	return GetNodes()[i];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory** CIndexTreeNodeMemory::GetNodes(void)
{
	return (CIndexTreeNodeMemory**)GetNodesMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode)
{
	int						i;
	int						iNumNodes;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();

	iNumNodes = GetNumIndexes();
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
unsigned char CIndexTreeNodeMemory::FindNextFirstIndex(void)
{
	unsigned char			i;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;

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
unsigned char CIndexTreeNodeMemory::FindPrevLastIndex(void)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();
	for (i = (int)(muiLastIndex - muiFirstIndex)-1; i >= 0; i--)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return muiFirstIndex + (unsigned char)i;
		}
	}
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::FindIndex(CIndexTreeNodeMemory* pcChild)
{
	int						i;
	CIndexTreeNodeMemory*	pcChildTest;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();
	for (i = 0; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChildTest = apcChildren[i];
		if (pcChild == pcChildTest)
		{
			return (unsigned char)i + muiFirstIndex;
		}
	}

	//This should never be hit.
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Contain(unsigned char uiIndex)
{
	//Contain assumes that the memory this node resides in has already been sized large enough.

	if (mbNodesEmpty == TRUE)
	{
		mbNodesEmpty = FALSE;
		ClearOnlyNode(uiIndex, 0);
		return;
	}

	if (uiIndex < muiFirstIndex)
	{
		MoveNodesRight(uiIndex, 0);
	}
	else if (uiIndex > muiLastIndex)
	{
		ClearLastNodes(uiIndex, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::Uncontain(unsigned char uiIndex)
{
	unsigned char	uiNextFirstIndex;

	if ((uiIndex != muiFirstIndex) && (uiIndex != muiLastIndex))
	{
		return FALSE;
	}
	else if (muiFirstIndex == muiLastIndex)
	{
		mbNodesEmpty = TRUE;
		ClearOnlyNode(0, 0);
		return TRUE;
	}

	if (uiIndex == muiFirstIndex)
	{
		uiNextFirstIndex = FindNextFirstIndex();
		MoveNodesLeft(uiNextFirstIndex);
	}
	else if (uiIndex == muiLastIndex)
	{
		muiLastIndex = FindPrevLastIndex();
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::SetChildsParent(void)
{
	int						i;
	int						iNumNodes;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;

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
int CIndexTreeNodeMemory::NumInitialisedIndexes(void)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;
	int						iCount;

	if ((mbNodesEmpty == TRUE) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	apcChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::ValidateNodesEmpty(void)
{
	int		iCount;
	BOOL	bCountEmpty;

	iCount = NumInitialisedIndexes();

	bCountEmpty = (iCount == 0);
	if (mbNodesEmpty == bCountEmpty)
	{
		return TRUE;
	}
	else
	{
		if (mbNodesEmpty)
		{
			gcLogger.Error2(__METHOD__, " Child nodes marked as empty but ", IntToString(iCount), " are allocated.", NULL);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child nodes marked as not empty but none are allocated.", NULL);
		}
		return FALSE;
	}
}

