#include "IndexTree.h"
#include "IntegerHelper.h"
#include "IndexTreeNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiFirstIndex, size uiLastIndex, size uiDataSize, size iClearValue, size uiIndexInParent)
{
	size	uiSize;

	muiMagic = INDEX_TREE_NODE_MAGIC;
	mpcIndexTree = pcIndexTree;
	muiFirstIndex = (uint8)uiFirstIndex;
	muiLastIndex = (uint8)uiLastIndex;
	msFlags = 0;
	muiIndexInParent = (uint8)uiIndexInParent;
	mpcParent = pcParent;

	if (uiDataSize != 0)
	{
		SetData(true);
		GetNodeData()->Init(uiDataSize);
	}
	uiSize = (uiLastIndex - uiFirstIndex + 1) * SizeofNodePtr();
	memset(GetNodesMemory(), iClearValue, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiIndexInParent)
{
	muiMagic = INDEX_TREE_NODE_MAGIC;
	mpcIndexTree = pcIndexTree;
	mpcParent = pcParent;
	muiFirstIndex = 0;
	muiLastIndex = 0;
	msFlags = 0;
	muiIndexInParent = (uint8)uiIndexInParent;
	SetNodesEmpty(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetDataSize(void)
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
size CIndexTreeNode::SizeofNode(void)
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
size CIndexTreeNode::SizeofNodeAndData(void)
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
size CIndexTreeNode::SizeofNodePtr(void)
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
size CIndexTreeNode::GetIndexInParent(void)
{
	return muiIndexInParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetFlags(void)
{
	return msFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetTransientFlags(void)
{
	return msFlags & INDEX_TREE_NODE_TRANSIENT_FLAGS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTree* CIndexTreeNode::GetIndexTree(void)
{
	return mpcIndexTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::CalculateRequiredNodeSizeForIndex(size uiIndex)
{
	size	uiSize;
	size	iRequiredIndices;

	if (HasNodes())
	{
		iRequiredIndices = GetAdditionalIndexes(uiIndex);
		uiSize = SizeofNodeAndData() + iRequiredIndices * SizeofNodePtr();
		return uiSize;
	}
	else
	{
		uiSize = mpcIndexTree->CalculateNodeSize(1, GetDataSize());
		return uiSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::CalculateRequiredNodeSizeForData(size uiDataSize)
{
	size	uiSize;
	size	iExistingIndices;

	iExistingIndices = NumIndexes();
	uiSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, uiDataSize);

	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::CalculateRequiredNodeSizeForCurrent(void)
{
	size	uiSize;
	size	iExistingIndices;

	iExistingIndices = NumIndexes();
	uiSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, GetDataSize());

	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::Contain(size uiIndex, size iClearValue)
{
	//Contain assumes that the memory this node resides in has already been sized large enough.
	if (!HasNodes())
	{
		SetNodesEmpty(false);
		ClearOnlyNode(uiIndex, iClearValue);
		return;
	}

	if (uiIndex < muiFirstIndex)
	{
		MoveNodesRight(uiIndex, iClearValue);
	}
	else if (uiIndex > muiLastIndex)
	{
		ClearLastNodes(uiIndex, iClearValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearOnlyNode(size uiIndex, size iClearValue)
{
	size	uiSize;
	void*	pvNodes;

	pvNodes = GetNodesMemory();
	muiFirstIndex = (uint8)uiIndex;
	muiLastIndex = (uint8)uiIndex;
	uiSize = SizeofNodePtr();
	memset(pvNodes, iClearValue, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::MoveNodesLeft(size uiNextFirstIndex)
{
	size	uiSize;
	void*	pvSource;
	void*	pvNodes;
	size	iNewNumIndexes;

	pvNodes = GetNodesMemory();

	iNewNumIndexes = NumIndexes(uiNextFirstIndex, muiLastIndex);
	uiSize = (uiNextFirstIndex - muiFirstIndex) * SizeofNodePtr();
	pvSource = (CIndexTreeNode**)RemapSinglePointer(pvNodes, uiSize);
	memmove(pvNodes, pvSource, iNewNumIndexes * SizeofNodePtr());
	muiFirstIndex = (uint8)uiNextFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::MoveNodesRight(size uiNewFirstIndex, size iClearValue)
{
	size		uiSize;
	void*		pvNodes;
	void*		pvDest;
	size		uiOldNumIndexes;

	pvNodes = GetNodesMemory();
	uiOldNumIndexes = NumIndexes();

	uiSize = (muiFirstIndex - uiNewFirstIndex) * SizeofNodePtr();
	pvDest = (CIndexTreeNode**)RemapSinglePointer(pvNodes, uiSize);
	memmove(pvDest, pvNodes, uiOldNumIndexes * SizeofNodePtr());
	memset(pvNodes, iClearValue, uiSize);
	muiFirstIndex = (uint8)uiNewFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearLastNodes(size uiNewLastIndex, size iClearValue)
{
	size		uiOldNumIndexes;
	void*		pvDest;
	size		uiSize;
	void*		pvNodes;

	pvNodes = GetNodesMemory();
	uiOldNumIndexes = NumIndexes();

	uiSize = (uiNewLastIndex - muiLastIndex) * SizeofNodePtr();
	pvDest = (CIndexTreeNode**)RemapSinglePointer(pvNodes, uiOldNumIndexes * SizeofNodePtr());
	memset(pvDest, iClearValue, uiSize);
	muiLastIndex = (uint8)uiNewLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::NumIndexes(void)
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
size CIndexTreeNode::NumIndexes(size uiFirstIndex, size uiLastIndex)
{
	return (uiLastIndex - uiFirstIndex) + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetAdditionalIndexes(size uiIndex)
{
	size	uiFirstIndex;
	size	uiLastIndex;

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
bool CIndexTreeNode::IsEmpty(void)
{
	if (HasData())
	{
		return false;
	}

	return !HasNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::IsDirty(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DIRTY_NODE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::IsPathDirty(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DIRTY_PATH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::IsDeleted(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DELETED_NODE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::IsPathDeleted(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DELETED_PATH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::IsMagic(void)
{
	return muiMagic == INDEX_TREE_NODE_MAGIC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::HasFlags(size sFlags)
{
	//True if any flag matches (including all flags matching).
	return FixBool(msFlags & sFlags);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearFlags(size sFlags)
{
	SetFlagByte(&msFlags, (uint8)sFlags, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetFirstIndex(void)
{
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::GetLastIndex(void)
{
	return muiLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::HasNodes(void)
{
	return !FixBool(msFlags & INDEX_TREE_NODE_FLAG_NODES_EMPTY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::HasData(void)
{
	return FixBool(msFlags & INDEX_TREE_NODE_FLAG_DATA);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetData(bool bHasData)
{
	return SetFlagByte(&msFlags, INDEX_TREE_NODE_FLAG_DATA, bHasData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeNode::NumAllocatedNodes(void)
{
	return NumIndexes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeNode::ContainsIndex(size uiIndex)
{
	if (!HasNodes())
	{
		return false;
	}

	if ((uiIndex >= muiFirstIndex) && (uiIndex <= muiLastIndex))
	{
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
void CIndexTreeNode::SetNodesEmpty(bool bEmpty)
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
void CIndexTreeNode::SetDirtyNode(bool bDirty)
{
	SetFlagByte(&msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, bDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDeletedNode(bool bDeleted)
{
	SetFlagByte(&msFlags, INDEX_TREE_NODE_FLAG_DELETED_NODE, bDeleted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDirtyPath(bool bDirty)
{
	SetFlagByte(&msFlags, INDEX_TREE_NODE_FLAG_DIRTY_PATH, bDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetDeletedPath(bool bDeleted)
{
	SetFlagByte(&msFlags, INDEX_TREE_NODE_FLAG_DELETED_PATH, bDeleted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::SetData(void* pvData, size uiDataSize)
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
void CIndexTreeNode::ChangeDataSize(size uiSize)
{
	size				tIndexSize;
	void*				apcChildren;
	int					iDataSizeDiff;
	void*				apcMovedChildren;
	bool				bHasData;
	size				iOldDataSize;

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
			SetData(true);
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
				iDataSizeDiff = -(int)((iOldDataSize + (mpcIndexTree->SizeofDataNode() - mpcIndexTree->SizeofNode())));

				apcMovedChildren = RemapSinglePointer(apcChildren, iDataSizeDiff);
				memmove(apcMovedChildren, apcChildren, tIndexSize);
			}
			SetData(false);
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
void CIndexTreeNode::Print(CChars* psz, bool bHex)
{
	if (IsEmpty())
	{
		psz->Append("Empty");
	}
	else
	{
		if (!bHex)
		{
			psz->Append((uint16)GetFirstIndex());
			psz->Append(":");
			psz->Append((uint16)GetLastIndex());
		}
		else
		{
			psz->Append((int32)GetFirstIndex(), (uint16)16);
			psz->Append(":");
			psz->Append((int32)GetLastIndex(), (uint16)16);
		}
	}

	if (GetDataSize() > 0)
	{
		psz->Append(" (");
		psz->Append((uint16)GetDataSize());
		psz->Append(")");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeNode::GetFlagsString(CChars* psz)
{
	bool	bAppendComma;

	bAppendComma = psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, "DIRTY_NODE");
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_PATH, "DIRTY_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_NODE, "DELETED_NODE", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_PATH, "DELETED_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DATA, "HAS_DATA", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_NODES_EMPTY, "NODES_EMPTY", bAppendComma);
	return psz->Text();
}

