#include "BaseLib/IntegerHelper.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "IndexTreeNodeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, CFileDataIndex cFileIndex, unsigned char uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, unsigned char uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiIndexInParent);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::SetFileIndex(int iFile, unsigned int uiIndex)
{
	mcFileIndex.Init(iFile, uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileDataIndex* CIndexTreeNodeFile::GetFileIndex(void)
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
BOOL CIndexTreeNodeFile::SetFile(unsigned char uiIndex, CFileDataIndex cFileNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Init(cFileNode);
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
BOOL CIndexTreeNodeFile::SetMemory(unsigned char uiIndex, CIndexTreeNodeFile* pcNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Init(pcNode);
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
BOOL CIndexTreeNodeFile::ClearIndex(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex - muiFirstIndex)->Clear();
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
BOOL CIndexTreeNodeFile::ClearIndexAndUncontain(unsigned char uiIndex)
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
CIndexTreeChildNode* CIndexTreeNodeFile::GetFirstNode(void)
{
	if (HasNodes())
	{
		return GetNodes();
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
void CIndexTreeNodeFile::RemapChildNodes(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNewNode)
{
	CIndexTreeChildNode*	acChildren;
	CIndexTreeChildNode*	pcChild;
	unsigned char			uiIndex;

	acChildren = GetNodes();
	uiIndex = pcNewNode->muiIndexInParent - muiFirstIndex;
	pcChild = &acChildren[uiIndex];
	if (pcChild->IsMemory())
	{
		pcChild->u.mpcMemory = pcNewNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeFile::FindNextFirstIndex(void)
{
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
BOOL CIndexTreeNodeFile::Uncontain(unsigned char uiIndex)
{
	unsigned char	uiNextFirstIndex;

	if ((uiIndex != muiFirstIndex) && (uiIndex != muiLastIndex))
	{
		return FALSE;
	}

	if (muiFirstIndex == muiLastIndex)
	{
		SetNodesEmpty(TRUE);
		muiFirstIndex = 0;
		muiLastIndex = 0;
		return TRUE;
	}

	if (uiIndex == muiFirstIndex)
	{
		uiNextFirstIndex = FindNextFirstIndex();
		MoveNodesLeft(uiNextFirstIndex);
		return TRUE;
	}

	if (uiIndex == muiLastIndex)
	{
		muiLastIndex = FindPrevLastIndex();
		return TRUE;
	}

	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::SetChildrensParent(void)
{
	int						i;
	int						iNumNodes;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;

	iNumNodes = NumIndexes();
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
int CIndexTreeNodeFile::NumValidIndexes(void)
{
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	int						iCount;
	int						iLength;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	acChildren = GetNodes();
	iLength = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= iLength; i++)
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
int CIndexTreeNodeFile::NumValidMemoryIndexes(void)
{
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	int						iCount;
	int						iLength;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	acChildren = GetNodes();
	iLength = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= iLength; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsMemory())
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
CIndexTreeNodeFile* CIndexTreeNodeFile::GetValidMemoryNode(int iIndex)
{
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	int						iCount;
	int						iLength;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return NULL;
	}

	iCount = 0;
	acChildren = GetNodes();
	iLength = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= iLength; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsMemory())
		{
			if (iCount == iIndex)
			{
				return pcChild->u.mpcMemory;
			}
			iCount++;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeFile::CalculateNodeSize(void)
{
	return (2 * sizeof(int)) + sizeof(unsigned short) + (4 * sizeof(unsigned char)) + (NumIndexes() * (sizeof(int) + sizeof(unsigned int)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeFile::CalculateDataBufferSize(CIndexTreeFileCallback* pcCallback)
{
	unsigned short uiDataSize;

	uiDataSize = GetDataSize();
	return pcCallback->IndexTreeDataSize(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeFile::CalculateFileSize(CIndexTreeFileCallback* pcCallback)
{
	return CalculateNodeSize() + CalculateDataBufferSize(pcCallback);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeFile::WriteToBuffer(void* pvBuffer, int iBufferSize, CIndexTreeFileCallback* pcCallback)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iNumNodes;
	CIndexTreeChildNode*	pcChild;
	CFileDataIndex			cEmptyIndex;
	CFileDataIndex*			pcChildIndex;
	int						iFileSize;
	void*					pvDataBuffer;
	void*					pvSourceData;
	int						iFileDataSize;
	int						iNodeSize;
	unsigned short			uiDataSize;

	iFileDataSize = CalculateDataBufferSize(pcCallback);
	iNodeSize = CalculateNodeSize();
	iFileSize = iFileDataSize + iNodeSize;
	if (iBufferSize < iFileSize)
	{
		gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile size [", IntToString(iFileSize), "] to buffer size [", IntToString(iBufferSize), "].  Buffer to small.", NULL);
		return 0;
	}

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;
	uiDataSize = GetDataSize();

	*((int*)&pucMemory[iPos]) = iFileSize;  iPos += sizeof(int);
	*((int*)&pucMemory[iPos]) = iFileDataSize;  iPos += sizeof(int);
	*((short*)&pucMemory[iPos]) = uiDataSize;  iPos += sizeof(unsigned short);

	pucMemory[iPos] = muiFirstIndex;  iPos += sizeof(unsigned char);
	pucMemory[iPos] = muiLastIndex;  iPos += sizeof(unsigned char);
	pucMemory[iPos] = muiIndexInParent;  iPos += sizeof(unsigned char);
	pucMemory[iPos] = msFlags & INDEX_TREE_NODE_TRANSIENT_FLAGS_MASK;  iPos += sizeof(unsigned char);

	pvSourceData = GetDataPtr();
	if (pvSourceData != NULL)
	{
		pvDataBuffer = &pucMemory[iPos];
		pcCallback->IndexTreeWriteData(pvDataBuffer, pvSourceData, iFileDataSize, uiDataSize);
		iPos += iFileDataSize;
	}

	cEmptyIndex.Init();
	iNumNodes = NumIndexes();
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
		*((unsigned int*)&pucMemory[iPos]) = pcChildIndex->muiIndex;  iPos += sizeof(unsigned int);
	}

	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeFile::InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileCallback* pcCallback)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iNumNodes;
	CIndexTreeChildNode*	pcChild;
	int						iFileSize;
	int						iFile;
	unsigned int			ulliFilePos;
	unsigned char			uiFirstIndex;
	unsigned char			uiLastIndex;
	unsigned char			uiIndexInParent;
	void*					pvDest;
	int						iFileDataSize;
	unsigned short			uiDataSize;

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;

	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	if (iMaxBufferSize < iFileSize)
	{
		gcLogger.Error2(__METHOD__, " Node buffer size [", IntToString(iMaxBufferSize), "] did not match node size in file [", IntToString(iFileSize), "].", NULL);
		return 0;
	}

	iFileDataSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	uiDataSize = *((unsigned short*)&pucMemory[iPos]);  iPos += sizeof(unsigned short);

	uiFirstIndex = pucMemory[iPos];  iPos++;
	uiLastIndex = pucMemory[iPos];  iPos++;
	uiIndexInParent = pucMemory[iPos];  iPos++;
	msFlags = pucMemory[iPos];  iPos++;

	if ((uiFirstIndex != muiFirstIndex) || (uiLastIndex != muiLastIndex) || (muiIndexInParent != uiIndexInParent))
	{
		gcLogger.Error2(__METHOD__, " Node fields for children  [", IntToString(muiFirstIndex), ", ", IntToString(muiLastIndex), ", ", IntToString(muiIndexInParent),  "] do not match node fields read [", IntToString(uiFirstIndex), ", ", IntToString(uiLastIndex), ", ", IntToString(uiIndexInParent), "].", NULL);
		return 0;
	}

	pvDest = GetDataPtr();
	if (pvDest != NULL)
	{
		pcCallback->IndexTreeReadData(pvDest, &pucMemory[iPos], uiDataSize, iFileDataSize);
		iPos += iFileDataSize;
	}

	iNumNodes = NumIndexes();
	for (int i = 0; i < iNumNodes; i++)
	{
		iFile = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
		ulliFilePos = *((unsigned int*)&pucMemory[iPos]);  iPos += sizeof(unsigned int);

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
	BOOL	bNodesEmpty;

	iCount = NumValidIndexes();

	bCountEmpty = (iCount == 0);
	bNodesEmpty = !HasNodes();
	if (bNodesEmpty == bCountEmpty)
	{
		return TRUE;
	}
	else
	{
		if (bNodesEmpty)
		{
			return gcLogger.Error2(__METHOD__, " Child nodes marked as empty but ", IntToString(iCount), " are allocated.", NULL);
		}
		else
		{
			return gcLogger.Error2(__METHOD__, " Child nodes marked as not empty but none are allocated.", NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeFile::HasOnlyFileNodes(void)
{
	int						iNumNodes;
	int						i;
	CIndexTreeChildNode*	pcChild;

	iNumNodes = NumIndexes();
	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = GetNode(i);
		if (!(pcChild->IsFile() || pcChild->IsUnallocated()))
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
BOOL CIndexTreeNodeFile::HasChildWithFlags(unsigned char uiFlags)
{
	int						iNumNodes;
	int						i;
	CIndexTreeChildNode*	pcChild;

	iNumNodes = NumIndexes();
	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = GetNode(i);
		if (pcChild->IsMemory())
		{
			if (pcChild->u.mpcMemory->HasFlags(uiFlags))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeFile::HasFile(void)
{
	return mcFileIndex.HasFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeFile::ConvertToFileNode(CIndexTreeNodeFile* pcNode)
{
	unsigned char			c;
	CIndexTreeChildNode*	pcNodeReference;

	c = pcNode->GetIndexInParent();
	pcNodeReference = Get(c);
	if (pcNodeReference->IsMemory())
	{
		if (pcNode->HasFile())
		{
			if (pcNode->IsDirty())
			{
				gcLogger.Error2(__METHOD__, " Cannot convert memory node to file node, node is dirty.", NULL);
				return FALSE;
			}
			else if (pcNode->IsDeleted())
			{
				gcLogger.Error2(__METHOD__, " Cannot convert memory node to file node, node is deleted.", NULL);
				return FALSE;
			}

			pcNodeReference->Init(pcNode->GetFileIndex());
			return TRUE;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot convert memory node to file node, node has not been written.", NULL);
			return FALSE;
		}
	}
	else if (pcNodeReference->IsFile())
	{
		return TRUE;
	}
	else if (pcNodeReference->IsUnallocated())
	{
		gcLogger.Error2(__METHOD__, " Cannot convert memory node to file node, node is not allocated.", NULL);
		return FALSE;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot convert to file node, node is corrupt.", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Print(CChars* psz, BOOL bHex)
{
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	CIndexTreeNodeFile*		pcMemoryChild;

	CIndexTreeNode::Print(psz, bHex);

	if (((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0)))
	{
		return;
	}

	psz->Append(" ");

	acChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = &acChildren[i];
		if (pcChild->IsValid())
		{
			if (pcChild->IsMemory())
			{
				pcMemoryChild = pcChild->u.mpcMemory;
				if (pcMemoryChild->GetFileIndex()->HasFile())
				{
					psz->Append("X");
				}
				else
				{
					psz->Append("x");
				}
			}
			else
			{
				psz->Append("o");
			}
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
void CIndexTreeNodeFile::Dump(void)
{
	CChars	sz;

	sz.Init();

	Print(&sz, FALSE);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


