#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "IndexTreeNodeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileIndex cFileIndex)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, -1);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.mulliFilePos);
}


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
void CIndexTreeNodeFile::SetFileIndex(int iFile,  filePos ulliFilePos)
{
	mcFileIndex.Init(iFile, ulliFilePos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIndex* CIndexTreeNodeFile::GetFileIndex(void)
{
	return &mcFileIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeChildNode* CIndexTreeNodeFile::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return (CIndexTreeChildNode*)GetNode(uiIndex - muiFirstIndex);
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
void CIndexTreeNodeFile::Set(unsigned char uiIndex, CFileIndex cFileNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Init(cFileNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Set(unsigned char uiIndex, CIndexTreeNodeFile* pcNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Init(pcNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeFile::Clear(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Clear();
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
CIndexTreeChildNode* CIndexTreeNodeFile::GetNode(int i)
{
	return &(GetNodes()[i]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeChildNode* CIndexTreeNodeFile::GetNodes(void)
{
	return (CIndexTreeChildNode*)GetNodesMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::RemapChildNodes(CIndexTreeChildNode* pcOldNode, CIndexTreeChildNode* pcNewNode)
{
	//Doubtful method works as expected.

	int						i;
	int						iNumNodes;
	CIndexTreeChildNode*	acChildren;

	acChildren = GetNodes();

	iNumNodes = GetNumIndexes();
	for (i = 0; i < iNumNodes; i++)
	{
		if (&acChildren[i] == pcOldNode)
		{
			acChildren[i] = *pcNewNode;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeFile::FindNextFirstIndex(void)
{
	//Doubtful method works as expected.

	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;

	acChildren = GetNodes();
	for (i = 1; i <= (int)muiLastIndex - (int)muiFirstIndex; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsValid())
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
	//Doubtful method works as expected.

	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;

	acChildren = GetNodes();
	for (i = ((int)muiLastIndex - (int)muiFirstIndex) - 1; i >= 0; i--)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsValid())
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
unsigned char CIndexTreeNodeFile::FindIndex(CIndexTreeChildNode* pcChild)
{
	//Doubtful method works as expected.

	int						i;
	CIndexTreeChildNode*	pcChildTest;
	CIndexTreeChildNode*	acChildren;

	acChildren = GetNodes();
	for (i = 0; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChildTest = &acChildren[i];
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
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;

	iNumNodes = GetNumIndexes();
	acChildren = GetNodes();

	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsMemory())
		{
			pcChild->u.mpcMemory->mpcParent = this;
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
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	int						iCount;

	if ((mbNodesEmpty == TRUE) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	acChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsValid())
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
int CIndexTreeNodeFile::CalculateBufferSize(void)
{
	return sizeof(int) + (4 * sizeof(unsigned char)) + muiDataSize + (GetNumIndexes() * sizeof(CFileIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeFile::WriteToBuffer(void* pvBuffer, int iBufferSize)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iNumNodes;
	CIndexTreeChildNode*	pcChild;
	CFileIndex				cEmptyIndex;
	CFileIndex*				pcChildIndex;
	int						iFileSize;

	iFileSize = CalculateBufferSize();
	if (iBufferSize < iFileSize)
	{
		return 0;
	}

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;

	*((int*)&pucMemory[iPos]) = iFileSize;  iPos += sizeof(int);

	pucMemory[iPos] = muiFirstIndex;  iPos++;
	pucMemory[iPos] = muiLastIndex;  iPos++;
	pucMemory[iPos] = muiDataSize;  iPos++;
	pucMemory[iPos] = mbNodesEmpty;  iPos++;

	if (HasObject())
	{
		//This assumes (obviously) that the object is persistable.  i.e. Not a pointer.
		memcpy_fast(&pucMemory[iPos], GetObjectPtr(), muiDataSize);
		iPos += muiDataSize;
	}

	cEmptyIndex.Init();
	iNumNodes = GetNumIndexes();
	for (int i = 0; i < iNumNodes; i++)
	{
		pcChild = GetNode(i);
		if (pcChild->IsFile())
		{
			pcChildIndex = &pcChild->u.mcFile;
		}
		else if (pcChild->IsMemory())
		{
			pcChildIndex = pcChild->u.mpcMemory->GetFileIndex();
		}
		else
		{
			pcChildIndex = &cEmptyIndex;
		}
		*((int*)&pucMemory[iPos]) = pcChildIndex->miFile;  iPos += sizeof(int);
		*((filePos*)&pucMemory[iPos]) = pcChildIndex->mulliFilePos;  iPos += sizeof(filePos);
	}

	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeFile::Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeFile* pcParent, void* pvBuffer, int iBufferSize, CFileIndex cFileIndex)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iNumNodes;
	CIndexTreeChildNode*	pcChild;
	int						iFileSize;
	int						iFile;
	filePos					ulliFilePos;

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;

	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	if (iBufferSize < iFileSize)
	{
		return 0;
	}

	muiFirstIndex = pucMemory[iPos];  iPos++;
	muiLastIndex = pucMemory[iPos];  iPos++;
	muiDataSize = pucMemory[iPos];  iPos++;
	mbNodesEmpty = pucMemory[iPos];  iPos++;

	if (HasObject())
	{
		memcpy_fast(GetObjectPtr(), &pucMemory[iPos], muiDataSize);
		iPos += muiDataSize;
	}

	iNumNodes = GetNumIndexes();
	for (int i = 0; i < iNumNodes; i++)
	{
		iFile = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
		ulliFilePos = *((filePos*)&pucMemory[iPos]);  iPos += sizeof(filePos);

		pcChild = GetNode(i);
		if (iFile != -1)
		{
			pcChild->Init(iFile, ulliFilePos);
		}
		else
		{
			pcChild->Clear();
		}
	}

	return iPos;
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
