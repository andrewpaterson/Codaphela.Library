#include "IntegerHelper.h"
#include "Logger.h"
#include "LogString.h"
#include "IndexTreeFile.h"
#include "IndexTreeNodeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, CFileDataIndex cFileIndex, size uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, size uiDataSize, size uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiFirstIndex, size uiLastIndex, size uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiFirstIndex, uiLastIndex, 0, INDEX_TREE_FILE_NODE_UNALLOCATED, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, CFileDataIndex cFileIndex, size uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiIndexInParent);
	mcFileIndex.Init(cFileIndex.miFile, cFileIndex.muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::Init(CIndexTree* pcIndexTree, CIndexTreeNodeFile* pcParent, size uiIndexInParent)
{
	CIndexTreeNode::Init(pcIndexTree, pcParent, uiIndexInParent);
	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeFile::SetFileIndex(uint32 iFile, uint32 uiIndex)
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
CIndexTreeChildNode* CIndexTreeNodeFile::Get(size uiIndex)
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
bool CIndexTreeNodeFile::SetFile(size uiIndex, CFileDataIndex cFileNode)
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
bool CIndexTreeNodeFile::SetMemory(size uiIndex, CIndexTreeNodeFile* pcNode)
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
bool CIndexTreeNodeFile::ClearIndex(size uiIndex)
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
bool CIndexTreeNodeFile::ClearIndexAndUncontain(size uiIndex)
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
CIndexTreeChildNode* CIndexTreeNodeFile::GetNode(size i)
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
	size					uiIndex;

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
size CIndexTreeNodeFile::FindNextFirstIndex(void)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	size					uiNumIndices;

	acChildren = GetNodes();
	uiNumIndices = muiLastIndex - muiFirstIndex;
	for (i = 1; i <= uiNumIndices; i++)
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
size CIndexTreeNodeFile::FindPrevLastIndex(void)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;

	acChildren = GetNodes();
	if (muiLastIndex > muiFirstIndex)
	{
		i = muiLastIndex - muiFirstIndex;
		do
		{
			i--;
			pcChild = &acChildren[i];
			if (pcChild->IsValid())
			{
				return muiFirstIndex + i;
			}
		}
		while (i != 0);
	}

	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::Uncontain(size uiIndex)
{
	size	uiNextFirstIndex;

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
		muiLastIndex = (uint8)FindPrevLastIndex();
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
	size					i;
	size					iNumNodes;
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
size CIndexTreeNodeFile::NumValidIndexes(void)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	size					iCount;
	size					uiNumIndices;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	acChildren = GetNodes();
	uiNumIndices = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= uiNumIndices; i++)
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
size CIndexTreeNodeFile::NumValidMemoryIndexes(void)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	size					iCount;
	size					uiNumIndices;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return 0;
	}

	iCount = 0;
	acChildren = GetNodes();
	uiNumIndices = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= uiNumIndices; i++)
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
CIndexTreeNodeFile* CIndexTreeNodeFile::GetValidMemoryNode(size iIndex)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	size					iCount;
	size					uiNumIndices;

	if ((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0))
	{
		return NULL;
	}

	iCount = 0;
	acChildren = GetNodes();
	uiNumIndices = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= uiNumIndices; i++)
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
size CIndexTreeNodeFile::CalculateNodeSize(void)
{
	return (sizeof(size) * 2) + sizeof(uint16) + (sizeof(uint8) * 4) + (NumIndexes() * (sizeof(uint32) * 2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNodeFile::CalculateDataBufferSize(CIndexTreeFileDataCallback* pcCallback)
{
	size uiDataSize;

	uiDataSize = GetDataSize();
	return pcCallback->IndexTreeDataSize(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNodeFile::CalculateFileSize(CIndexTreeFileDataCallback* pcCallback)
{
	return CalculateNodeSize() + CalculateDataBufferSize(pcCallback);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNodeFile::WriteToBuffer(void* pvBuffer, size uiBufferSize, CIndexTreeFileDataCallback* pcCallback)
{
	uint8*					pucMemory;
	size					uiPos;
	size					iNumNodes;
	CIndexTreeChildNode*	pcChild;
	CFileDataIndex			cEmptyIndex;
	CFileDataIndex*			pcChildIndex;
	size					uiFileSize;
	void*					pvDataBuffer;
	void*					pvSourceData;
	size					uiFileDataSize;
	size					iNodeSize;
	size					uiDataSize;
	CIndexTreeChildNode*	apcChildren;
	size					i;

	uiFileDataSize = CalculateDataBufferSize(pcCallback);
	iNodeSize = CalculateNodeSize();
	uiFileSize = uiFileDataSize + iNodeSize;
	if (uiBufferSize < uiFileSize)
	{
		gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile size [", SizeToString(uiFileSize), "] to buffer size [", SizeToString(uiBufferSize), "].  Buffer to small.", NULL);
		return 0;
	}
	
	pucMemory = (uint8*)pvBuffer;
	uiPos = 0;
	uiDataSize = GetDataSize();

	*((size*)&pucMemory[uiPos]) = uiFileSize;  uiPos += sizeof(size);
	*((size*)&pucMemory[uiPos]) = uiFileDataSize;  uiPos += sizeof(size);
	*((int16*)&pucMemory[uiPos]) = (uint16)uiDataSize;  uiPos += sizeof(uint16);

	pucMemory[uiPos] = muiFirstIndex;  uiPos += sizeof(uint8);
	pucMemory[uiPos] = muiLastIndex;  uiPos += sizeof(uint8);
	pucMemory[uiPos] = muiIndexInParent;  uiPos += sizeof(uint8);
	pucMemory[uiPos] = msFlags & INDEX_TREE_NODE_TRANSIENT_FLAGS_MASK;  uiPos += sizeof(uint8);

	pvSourceData = GetDataPtr();
	if (pvSourceData != NULL)
	{
		pvDataBuffer = &pucMemory[uiPos];
		pcCallback->IndexTreeWriteData(pvDataBuffer, pvSourceData, uiFileDataSize, uiDataSize);
		uiPos += uiFileDataSize;
	}

	cEmptyIndex.Init();
	apcChildren = GetNodes();
	iNumNodes = NumIndexes();
	for (i = 0; i < iNumNodes; i++)
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
		*((uint32*)&pucMemory[uiPos]) = pcChildIndex->miFile;  uiPos += sizeof(uint32);
		*((uint32*)&pucMemory[uiPos]) = pcChildIndex->muiIndex;  uiPos += sizeof(uint32);
	}

	return uiPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNodeFile::InitFromBuffer(void* pvBuffer, size uiMaxBufferSize, CIndexTreeFileDataCallback* pcCallback)
{
	uint8*					pucMemory;
	size					uiPos;
	size					iNumNodes;
	CIndexTreeChildNode*	pcChild;
	size					uiFileSize;
	uint32					iFile;
	uint32					ulliFilePos;
	uint8					uiFirstIndex;
	uint8					uiLastIndex;
	uint8					uiIndexInParent;
	void*					pvDest;
	size					iFileDataSize;
	size					uiDataSize;
	CIndexTreeChildNode*	apcChildren;
	size					i;

	pucMemory = (uint8*)pvBuffer;
	uiPos = 0;

	uiFileSize = *((size*)&pucMemory[uiPos]);  uiPos += sizeof(size);
	if (uiMaxBufferSize < uiFileSize)
	{
		gcLogger.Error2(__METHOD__, " Node buffer size [", SizeToString(uiMaxBufferSize), "] did not match node size in file [", SizeToString(uiFileSize), "].", NULL);
		return 0;
	}

	iFileDataSize = *((size*)&pucMemory[uiPos]);  uiPos += sizeof(size);
	uiDataSize = *((uint16*)&pucMemory[uiPos]);  uiPos += sizeof(uint16);

	uiFirstIndex = pucMemory[uiPos];  uiPos++;
	uiLastIndex = pucMemory[uiPos];  uiPos++;
	uiIndexInParent = pucMemory[uiPos];  uiPos++;
	msFlags = pucMemory[uiPos];  uiPos++;

	if ((uiFirstIndex != muiFirstIndex) || (uiLastIndex != muiLastIndex) || (muiIndexInParent != uiIndexInParent))
	{
		gcLogger.Error2(__METHOD__, " Node fields for children  [", ByteToString(muiFirstIndex), ", ", ByteToString(muiLastIndex), ", ", ByteToString(muiIndexInParent),  "] do not match node fields read [", ByteToString(uiFirstIndex), ", ", ByteToString(uiLastIndex), ", ", ByteToString(uiIndexInParent), "].", NULL);
		return 0;
	}

	pvDest = GetDataPtr();
	if (pvDest != NULL)
	{
		pcCallback->IndexTreeReadData(pvDest, &pucMemory[uiPos], uiDataSize, iFileDataSize);
		uiPos += iFileDataSize;
	}

	iNumNodes = NumIndexes();
	apcChildren = GetNodes();
	for (i = 0; i < iNumNodes; i++)
	{
		iFile = *((uint32*)&pucMemory[uiPos]);  uiPos += sizeof(uint32);
		ulliFilePos = *((uint32*)&pucMemory[uiPos]);  uiPos += sizeof(uint32);

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

	return uiPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNodeFile::ValidateNodesEmpty(void)
{
	size	iCount;
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
	uint8					szKey[MAX_KEY_SIZE];
	size					iFirstIndex;

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
	size					iNumNodes;
	size					i;
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
bool CIndexTreeNodeFile::HasChildWithFlags(size uiFlags)
{
	size					iNumNodes;
	size					i;
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
	size					c;
	CIndexTreeChildNode*	pcNodeReference;
	uint8					szKey[MAX_KEY_SIZE];

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
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	CIndexTreeNodeFile*		pcMemoryChild;
	size					uiNumIndices;

	CIndexTreeNode::Print(psz, bHex);

	if (((!HasNodes()) && (muiLastIndex == 0) && (muiFirstIndex == 0)))
	{
		return;
	}

	psz->Append(" ");

	acChildren = GetNodes();
	uiNumIndices = muiLastIndex - muiFirstIndex;
	for (i = 0; i <= uiNumIndices; i++)
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

