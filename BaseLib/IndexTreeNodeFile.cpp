#include "IntegerHelper.h"
#include "Logger.h"
#include "LogString.h"
#include "IndexTreeFile.h"
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
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, uint16 uiDataSize, unsigned char uiIndexInParent)
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
		return (CIndexTreeChildNode*)GetNode(uiIndex);
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
bool CIndexTreeNodeFile::SetFile(unsigned char uiIndex, CFileDataIndex cFileNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex)->Init(cFileNode);
		return true;
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
bool CIndexTreeNodeFile::SetMemory(unsigned char uiIndex, CIndexTreeNodeFile* pcNode)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex)->Init(pcNode);
		return true;
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
bool CIndexTreeNodeFile::ClearIndex(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex)->Clear();
		return true;
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
bool CIndexTreeNodeFile::ClearIndexAndUncontain(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		GetNode(uiIndex)->Clear();

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
CIndexTreeChildNode* CIndexTreeNodeFile::GetNode(int i)
{
	return &(GetNodes()[i - muiFirstIndex]);
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
bool CIndexTreeNodeFile::Uncontain(unsigned char uiIndex)
{
	unsigned char	uiNextFirstIndex;

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
	return (2 * sizeof(int)) + sizeof(uint16) + (4 * sizeof(unsigned char)) + (NumIndexes() * (sizeof(int) + sizeof(unsigned int)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeFile::CalculateDataBufferSize(CIndexTreeFileDataCallback* pcCallback)
{
	uint16 uiDataSize;

	uiDataSize = GetDataSize();
	return pcCallback->IndexTreeDataSize(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeFile::CalculateFileSize(CIndexTreeFileDataCallback* pcCallback)
{
	return CalculateNodeSize() + CalculateDataBufferSize(pcCallback);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeFile::WriteToBuffer(void* pvBuffer, int iBufferSize, CIndexTreeFileDataCallback* pcCallback)
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
	uint16			uiDataSize;
	CIndexTreeChildNode*	apcChildren;

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
	*((int16*)&pucMemory[iPos]) = uiDataSize;  iPos += sizeof(uint16);

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
	apcChildren = GetNodes();
	iNumNodes = NumIndexes();
	for (int i = 0; i < iNumNodes; i++)
	{
		pcChild = &apcChildren[i];
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
int CIndexTreeNodeFile::InitFromBuffer(void* pvBuffer, int iMaxBufferSize, CIndexTreeFileDataCallback* pcCallback)
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
	uint16			uiDataSize;
	CIndexTreeChildNode*	apcChildren;

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;

	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	if (iMaxBufferSize < iFileSize)
	{
		gcLogger.Error2(__METHOD__, " Node buffer size [", IntToString(iMaxBufferSize), "] did not match node size in file [", IntToString(iFileSize), "].", NULL);
		return 0;
	}

	iFileDataSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	uiDataSize = *((uint16*)&pucMemory[iPos]);  iPos += sizeof(uint16);

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
	apcChildren = GetNodes();
	for (int i = 0; i < iNumNodes; i++)
	{
		iFile = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
		ulliFilePos = *((unsigned int*)&pucMemory[iPos]);  iPos += sizeof(unsigned int);

		pcChild = &apcChildren[i];
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
bool CIndexTreeNodeFile::ValidateNodesEmpty(void)
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
bool CIndexTreeNodeFile::ValidateParent(void)
{
	CIndexTreeNodeFile*		pcShouldBeThis;
	CIndexTreeChildNode*	pcShouldBeChild;
	char					szKey[MAX_KEY_SIZE];
	int						iFirstIndex;

	if (mpcParent == NULL)
	{
		return gcLogger.Error2(__METHOD__, " This node's parent should not be NULL.", NULL);
	}

	iFirstIndex = mpcParent->GetFirstIndex();
	pcShouldBeChild = ((CIndexTreeNodeFile*)mpcParent)->GetNode(muiIndexInParent);
	if (pcShouldBeChild->IsMemory())
	{
		pcShouldBeThis = pcShouldBeChild->u.mpcMemory;
		if (pcShouldBeThis != this)
		{
			((CIndexTreeFile*)mpcIndexTree)->GetNodeKey(this, szKey, MAX_KEY_SIZE);
			return gcLogger.Error2(__METHOD__, " This node is not the same as the parents child node for key [", szKey, "].", NULL);
		}
	}
	else
	{
		((CIndexTreeFile*)mpcIndexTree)->GetNodeKey(this, szKey, MAX_KEY_SIZE);
		return gcLogger.Error2(__METHOD__, " This node is not a memory node on the parents child node for key [", szKey, "].", NULL);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::HasOnlyFileNodes(void)
{
	int						iNumNodes;
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	apcChildren;

	iNumNodes = NumIndexes();
	apcChildren = GetNodes();
	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = &apcChildren[i];
		if (!(pcChild->IsFile() || pcChild->IsUnallocated()))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::HasChildWithFlags(unsigned char uiFlags)
{
	int						iNumNodes;
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	apcChildren;

	iNumNodes = NumIndexes();
	apcChildren = GetNodes();
	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = &apcChildren[i];
		if (pcChild->IsMemory())
		{
			if (pcChild->u.mpcMemory->HasFlags(uiFlags))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::HasFile(void)
{
	return mcFileIndex.HasFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::ConvertToFileNode(CIndexTreeNodeFile* pcNode)
{
	unsigned char			c;
	CIndexTreeChildNode*	pcNodeReference;
	char					szKey[MAX_KEY_SIZE];

	pcNode->ValidateParent();

	c = pcNode->GetIndexInParent();
	pcNodeReference = Get(c);
	if (pcNodeReference->IsMemory())
	{
		if (pcNode->HasFile())
		{
			if (pcNode->IsDirty())
			{
				gcLogger.Error2(__METHOD__, " Cannot convert memory node to file node, node is dirty.", NULL);
				return false;
			}
			else if (pcNode->IsDeleted())
			{
				((CIndexTreeFile*)mpcIndexTree)->GetNodeKey(pcNode, szKey, MAX_KEY_SIZE);
				gcLogger.Error2(__METHOD__, " Cannot convert memory node with key [", szKey, "] to file node, node is deleted.", NULL);
				return false;
			}

			pcNodeReference->Init(pcNode->GetFileIndex());
			return true;
		}
		else
		{
			((CIndexTreeFile*)mpcIndexTree)->GetNodeKey(pcNode, szKey, MAX_KEY_SIZE);
			gcLogger.Error2(__METHOD__, " Cannot convert memory node with key [", szKey, "] to file node, node has not been written.", NULL);
			return false;
		}
	}
	else if (pcNodeReference->IsFile())
	{
		return true;
	}
	else if (pcNodeReference->IsUnallocated())
	{
		((CIndexTreeFile*)mpcIndexTree)->GetNodeKey(pcNode, szKey, MAX_KEY_SIZE);
		gcLogger.Error2(__METHOD__, " Cannot convert memory node with key [", szKey, "] to file node, node is not allocated.", NULL);
		return false;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot convert to file node, node is corrupt.", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Print(CChars* psz, bool bHex)
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

	Print(&sz, false);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


