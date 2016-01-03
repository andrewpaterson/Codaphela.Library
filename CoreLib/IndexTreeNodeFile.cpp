#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "IndexTreeNodeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, -1);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileIndex cFileIndex)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.mulliFilePos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeNodeFile::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return (CIndexTreeNodeFile*)GetNode(uiIndex - muiFirstIndex);
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
void CIndexTreeNodeFile::Set(unsigned char uiIndex, CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile**	apcChildren;

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
BOOL CIndexTreeNodeFile::Clear(unsigned char uiIndex)
{
	CIndexTreeNodeFile**	apcChildren;

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
CIndexTreeNodeFile* CIndexTreeNodeFile::GetNode(int i)
{
	return GetNodes()[i];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile** CIndexTreeNodeFile::GetNodes(void)
{
	return (CIndexTreeNodeFile**)GetNodesMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode)
{
	int						i;
	int						iNumNodes;
	CIndexTreeNodeFile**	apcChildren;

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
unsigned char CIndexTreeNodeFile::FindNextFirstIndex(void)
{
	int						i;
	CIndexTreeNodeFile*	pcChild;
	CIndexTreeNodeFile**	apcChildren;

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
unsigned char CIndexTreeNodeFile::FindPrevLastIndex(void)
{
	int						i;
	CIndexTreeNodeFile*	pcChild;
	CIndexTreeNodeFile**	apcChildren;

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
unsigned char CIndexTreeNodeFile::FindIndex(CIndexTreeNodeFile* pcChild)
{
	int						i;
	CIndexTreeNodeFile*	pcChildTest;
	CIndexTreeNodeFile**	apcChildren;

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
void CIndexTreeNodeFile::Contain(unsigned char uiIndex)
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
BOOL CIndexTreeNodeFile::Uncontain(unsigned char uiIndex)
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
void CIndexTreeNodeFile::SetChildsParent(void)
{
	int						i;
	int						iNumNodes;
	CIndexTreeNodeFile*	pcChild;
	CIndexTreeNodeFile**	apcChildren;

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
int CIndexTreeNodeFile::NumInitialisedIndexes(void)
{
	int						i;
	CIndexTreeNodeFile*	pcChild;
	CIndexTreeNodeFile**	apcChildren;
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
BOOL CIndexTreeNodeFile::ValidateNodesEmpty(void)
{
	int		iCount;
	BOOL	bCountEmpty;

	iCount = NumInitialisedIndexes();

	bCountEmpty = iCount == 0;
	if (mbNodesEmpty == bCountEmpty)
	{
		return TRUE;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Child nodes marked as empty but ", IntToString(iCount) ," are allocated.", NULL);
		return FALSE;
	}
}

