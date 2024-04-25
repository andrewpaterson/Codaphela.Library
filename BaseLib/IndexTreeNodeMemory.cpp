#include "Logger.h"
#include "LogString.h"
#include "IndexTreeNodeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint8 uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, 0, uiIndexInParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiIndexInParent)
{
 	CIndexTreeNode::Init(pcIndexTree, pcParent, uiIndexInParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeNodeMemory::Get(uint8 uiIndex)
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
void CIndexTreeNodeMemory::Set(uint8 uiIndex, CIndexTreeNodeMemory* pcNode)
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
void CIndexTreeNodeMemory::ClearIndex(uint8 uiIndex)
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
bool CIndexTreeNodeMemory::ClearAndUncontain(uint8 uiIndex)
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
		return false;
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
	CIndexTreeNodeMemory**	apcChildren;
	uint8					uiIndex;

	apcChildren = GetNodes();
	uiIndex = pcNewNode->muiIndexInParent - muiFirstIndex;
	apcChildren[uiIndex] = pcNewNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::RemapDataLinks(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode)
{
	CIndexTreeDataNode*		pcDataNode;

	pcDataNode = GetNodeData();
	if (pcDataNode)
	{
		pcDataNode->RemapListNode();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Contain(uint8 uiIndex)
{
	CIndexTreeNode::Contain(uiIndex, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CIndexTreeNodeMemory::FindNextFirstIndex(void)
{
	uint8					i;
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
uint8 CIndexTreeNodeMemory::FindPrevLastIndex(void)
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
			return muiFirstIndex + i;
		}
	}
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CIndexTreeNodeMemory::FindIndex(CIndexTreeNodeMemory* pcChild)
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
			return (uint8)i + muiFirstIndex;
		}
	}

	//This should never be hit.
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeMemory::Uncontain(uint8 uiIndex)
{
	uint8	uiNextFirstIndex;

	if ((uiIndex != muiFirstIndex) && (uiIndex != muiLastIndex))
	{
		return false;
	}

	if (muiFirstIndex == muiLastIndex)
	{
		SetNodesEmpty(true);
		muiFirstIndex = 0;
		muiLastIndex = 0;
		return true;
	}

	if (uiIndex == muiFirstIndex)
	{
		uiNextFirstIndex = FindNextFirstIndex();
		MoveNodesLeft(uiNextFirstIndex);
		return true;
	}
	
	if (uiIndex == muiLastIndex)
	{
		muiLastIndex = FindPrevLastIndex();
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::SetChildrensParent(void)
{
	int						i;
	int						iNumNodes;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	iNumNodes = NumIndexes();
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
CIndexTreeNodeMemory* CIndexTreeNodeMemory::GetParent(void)
{
	return (CIndexTreeNodeMemory*)CIndexTreeNode::GetParent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeMemory::NumValidIndexes(void)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;
	int						iCount;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
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
bool CIndexTreeNodeMemory::ValidateNodesEmpty(void)
{
	int		iCount;
	bool	bCountEmpty;
	bool	bNodesEmpty;

	iCount = NumValidIndexes();

	bCountEmpty = (iCount == 0);
	bNodesEmpty = !HasNodes();
	if (bNodesEmpty == bCountEmpty)
	{
		return true;
	}
	else
	{
		if (bNodesEmpty)
		{
			gcLogger.Error2(__METHOD__, " Child nodes marked as empty but ", IntToString(iCount), " are allocated.", NULL);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child nodes marked as not empty but none are allocated.", NULL);
		}
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Print(CChars* psz, bool bHex)
{
	int						i;
	CIndexTreeNodeMemory*	pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	CIndexTreeNode::Print(psz, bHex);

	if (((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0)))
	{
		return;
	}

	psz->Append(" ");

	apcChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			psz->Append("x");
		}
		else
		{
			psz->Append(".");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Dump(void)
{
	CChars	sz;

	sz.Init();

	Print(&sz, false);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

