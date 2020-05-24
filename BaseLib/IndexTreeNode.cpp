#include "IndexTree.h"
#include "IntegerHelper.h"
#include "IndexTreeNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, int iClearValue, unsigned char uiIndexInParent)
{
	size_t tSize;

	muiMagic = INDEX_TREE_NODE_MAGIC;
	mpcIndexTree = pcIndexTree;
	muiFirstIndex = uiFirstIndex;
	muiLastIndex = uiLastIndex;
	msFlags = 0;
	muiIndexInParent = uiIndexInParent;
	mpcParent = pcParent;

	tSize = (uiLastIndex - uiFirstIndex + 1) * SizeofNodePtr();
	memset(GetNodesMemory(), iClearValue, tSize);

	if (uiDataSize != 0)
	{
		SetData(TRUE);
		GetNodeData()->Init(uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiIndexInParent)
{
	muiMagic = INDEX_TREE_NODE_MAGIC;
	mpcIndexTree = pcIndexTree;
	mpcParent = pcParent;
	muiFirstIndex = 0;
	muiLastIndex = 0;
	msFlags = 0;
	muiIndexInParent = uiIndexInParent;
	SetNodesEmpty(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeNode::GetDataSize(void)
{
	if (!HasData())
	{
		return 0;
	}
	else
	{
		return GetNodeData()->GetDataSize();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeNode::GetDataPtr(void)
{
	if (!HasData())
	{
		return NULL;
	}
	else
	{
		return RemapSinglePointer(this, mpcIndexTree->SizeofDataNode());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataNode* CIndexTreeNode::GetNodeData(void)
{
	if (!HasData())
	{
		return NULL;
	}
	else
	{
		return (CIndexTreeDataNode*)RemapSinglePointer(this, mpcIndexTree->SizeofNode());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::SizeofNode(void)
{
	if (HasData())
	{
		return mpcIndexTree->SizeofDataNode();
	}
	else
	{
		return mpcIndexTree->SizeofNode();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::SizeofNodeAndData(void)
{
	if (HasData())
	{
		return mpcIndexTree->SizeofDataNode() + GetNodeData()->GetDataSize();
	}
	else
	{
		return mpcIndexTree->SizeofNode();
	}
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
	if (HasNodes())
	{
		return RemapSinglePointer(this, SizeofNodeAndData());
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
CIndexTreeNode* CIndexTreeNode::GetParent(void)
{
	return mpcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetIndexInParent(void)
{
	return muiIndexInParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetFlags(void)
{
	return msFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::GetTransientFlags(void)
{
	return msFlags & INDEX_TREE_NODE_TRANSIENT_FLAGS;
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
		tSize = SizeofNodeAndData() + iRequiredIndices * SizeofNodePtr();
		return tSize;
	}
	else
	{
		tSize = mpcIndexTree->CalculateNodeSize(1, GetDataSize());
		return tSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForData(unsigned short uiDataSize)
{
	size_t	tSize;
	int		iExistingIndices;

	iExistingIndices = NumIndexes();
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

	iExistingIndices = NumIndexes();
	tSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, GetDataSize());

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

	iNewNumIndexes = NumIndexes(uiNextFirstIndex, muiLastIndex);
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
	uiOldNumIndexes = NumIndexes();

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
	uiOldNumIndexes = NumIndexes();

	tSize = (uiNewLastIndex - muiLastIndex) * SizeofNodePtr();
	pvDest = (CIndexTreeNode**)RemapSinglePointer(pvNodes, uiOldNumIndexes * SizeofNodePtr());
	memset(pvDest, iClearValue, tSize);
	muiLastIndex = uiNewLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::NumIndexes(void)
{
	if (!HasNodes())
	{
		return 0;
	}
	return NumIndexes(muiFirstIndex, muiLastIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNode::NumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex)
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
	if (HasData())
	{
		return FALSE;
	}

	return !HasNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsDirty(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DIRTY_NODE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsPathDirty(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DIRTY_PATH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsDeleted(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DELETED_NODE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsPathDeleted(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DELETED_PATH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::IsMagic(void)
{
	return muiMagic == INDEX_TREE_NODE_MAGIC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::HasFlags(unsigned char sFlags)
{
	//True if any flag matches (including all flags matching).
	return FixBool(msFlags & sFlags);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearFlags(unsigned char sFlags)
{
	SetFlag(&msFlags, sFlags, FALSE);
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
	return !FixBool(msFlags & INDEX_TREE_NODE_FLAG_NODES_EMPTY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::HasData(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DATA);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetData(BOOL bHasData)
{
	return SetFlag(&msFlags, INDEX_TREE_NODE_FLAG_DATA, bHasData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::NumAllocatedNodes(void)
{
	return (unsigned char)NumIndexes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::ContainsIndex(unsigned char uiIndex)
{
	if (!HasNodes())
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
void CIndexTreeNode::SetNodesEmpty(BOOL bEmpty)
{
	//If the value is true then OR it with dest.
	if (bEmpty)
	{
		msFlags |= INDEX_TREE_NODE_FLAG_NODES_EMPTY;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		msFlags &= ~INDEX_TREE_NODE_FLAG_NODES_EMPTY;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDirtyNode(BOOL bDirty)
{
	SetFlag(&msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, bDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDeletedNode(BOOL bDeleted)
{
	SetFlag(&msFlags, INDEX_TREE_NODE_FLAG_DELETED_NODE, bDeleted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDirtyPath(BOOL bDirty)
{
	SetFlag(&msFlags, INDEX_TREE_NODE_FLAG_DIRTY_PATH, bDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDeletedPath(BOOL bDeleted)
{
	SetFlag(&msFlags, INDEX_TREE_NODE_FLAG_DELETED_PATH, bDeleted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetData(void* pvData, unsigned short uiDataSize)
{
	if (GetDataSize() != uiDataSize)
	{
		ChangeDataSize(uiDataSize);
	}

	if (uiDataSize > 0)
	{
		if (pvData)
		{
			memcpy_fast(GetDataPtr(), pvData, uiDataSize);
		}
		else
		{
			memset_fast(GetDataPtr(), 0, uiDataSize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::RemoveData(void)
{
	if (GetDataSize() > 0)
	{
		ChangeDataSize(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ChangeDataSize(unsigned short uiSize)
{
	size_t				tIndexSize;
	void*				apcChildren;
	int					iDataSizeDiff;
	void*				apcMovedChildren;
	BOOL				bHasData;
	int					iOldDataSize;

	bHasData = HasData();
	iOldDataSize = GetDataSize();
	if (uiSize != 0)
	{
		if (bHasData)
		{
			//Had data and will have data.
			if (HasNodes())
			{
				tIndexSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();
				iDataSizeDiff = (int)uiSize - iOldDataSize;

				apcChildren = GetNodesMemory();
				apcMovedChildren = RemapSinglePointer(apcChildren, iDataSizeDiff);
				memmove(apcMovedChildren, apcChildren, tIndexSize);
			}
			GetNodeData()->SetDataSize(uiSize);
		}
		else
		{
			//Had NO data and will have data.
			if (HasNodes())
			{
				tIndexSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();
				iDataSizeDiff = (int)uiSize + (mpcIndexTree->SizeofDataNode() - mpcIndexTree->SizeofNode());

				apcChildren = GetNodesMemory();
				apcMovedChildren = RemapSinglePointer(apcChildren, iDataSizeDiff);
				memmove(apcMovedChildren, apcChildren, tIndexSize);
			}
			SetData(TRUE);
			GetNodeData()->Init(uiSize);
		}
	}
	else // uiSize == 0
	{
		if (bHasData)
		{
			//Had data and will NOT have data.
			apcChildren = GetNodesMemory();
			GetNodeData()->Kill();
			if (HasNodes())
			{
				tIndexSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();
				iDataSizeDiff = -((int)(iOldDataSize + (mpcIndexTree->SizeofDataNode() - mpcIndexTree->SizeofNode())));

				apcMovedChildren = RemapSinglePointer(apcChildren, iDataSizeDiff);
				memmove(apcMovedChildren, apcChildren, tIndexSize);
			}
			SetData(FALSE);
		}
		else
		{
			//Had NO data and will NOT have data.
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Print(CChars* psz, BOOL bHex)
{
	if (IsEmpty())
	{
		psz->Append("Empty");
	}
	else
	{
		if (!bHex)
		{
			psz->Append((int)GetFirstIndex());
			psz->Append(":");
			psz->Append((int)GetLastIndex());
		}
		else
		{
			psz->Append((int)GetFirstIndex(), 16);
			psz->Append(":");
			psz->Append((int)GetLastIndex(), 16);
		}
	}

	if (GetDataSize() > 0)
	{
		psz->Append(" (");
		psz->Append((int)GetDataSize());
		psz->Append(")");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeNode::GetFlagsString(CChars* psz)
{
	BOOL	bAppendComma;

	bAppendComma = psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, "DIRTY_NODE");
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_PATH, "DIRTY_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_NODE, "DELETED_NODE", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_PATH, "DELETED_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_NODES_EMPTY, "NODES_EMPTY", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DATA, "DATA", bAppendComma);
	return psz->Text();
}

