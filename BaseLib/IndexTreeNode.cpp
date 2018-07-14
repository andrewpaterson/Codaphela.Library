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
	muiDataSize = uiDataSize;
	msFlags = 0;
	muiIndexInParent = uiIndexInParent;
	mpcParent = pcParent;

	tSize = (uiLastIndex - uiFirstIndex + 1) * SizeofNodePtr();
	memset(GetNodesMemory(), iClearValue, tSize);
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
	muiDataSize = 0;
	msFlags = 0;
	muiIndexInParent = uiIndexInParent;
	SetNodesEmpty(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeNode::ObjectSize(void)
{
	return muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeNode::GetObjectPtr(void)
{
	if (muiDataSize == 0)
	{
		return NULL;
	}
	else
	{
		return RemapSinglePointer(this, SizeofNode());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::SizeofNode(void)
{
	return mpcIndexTree->SizeofNode();
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
	return RemapSinglePointer(this, SizeofNode() + muiDataSize);
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
		tSize = SizeofNode() + muiDataSize + iRequiredIndices * SizeofNodePtr();

		return tSize;
	}
	else
	{
		return CalculateRequiredNodeSizeForEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNode::CalculateRequiredNodeSizeForEmpty(void)
{
	return mpcIndexTree->CalculateNodeSize(1, muiDataSize);
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
	tSize = mpcIndexTree->CalculateNodeSize(iExistingIndices, muiDataSize);

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
	if (muiDataSize != 0)
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
	return !FixBool(msFlags & INDEX_TREE_NODE_FLAG_EMPTY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNode::HasObject(void)
{
	return muiDataSize != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNode::NumNodes(void)
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
		msFlags |= INDEX_TREE_NODE_FLAG_EMPTY;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		msFlags &= ~INDEX_TREE_NODE_FLAG_EMPTY;
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
BOOL CIndexTreeNode::SetObject(void* pvObject, unsigned short uiSize)
{
	if (muiDataSize != uiSize)
	{
		ChangeDataSize(uiSize);
	}

	if (pvObject)
	{
		memcpy_fast(GetObjectPtr(), pvObject, uiSize);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ClearObject(void)
{
	ChangeDataSize(0);
	SetDirtyNode(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNode::ChangeDataSize(unsigned short uiSize)
{
	size_t				tSize;
	void*				apcChildren;
	int					iDiff;
	void*				apcMovedChildren;

	if (HasNodes())
	{
		tSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();
		iDiff = (int)uiSize - (int)muiDataSize;

		apcChildren = GetNodesMemory();
		apcMovedChildren = RemapSinglePointer(apcChildren, iDiff);
		memmove(apcMovedChildren, apcChildren, tSize);
	}

	muiDataSize = uiSize;
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

	if (ObjectSize() > 0)
	{
		psz->Append(" (");
		psz->Append((int)ObjectSize());
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

	bAppendComma = psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, "INDEX_TREE_NODE_FLAG_DIRTY_NODE");
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_PATH, "INDEX_TREE_NODE_FLAG_DIRTY_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_NODE, "INDEX_TREE_NODE_FLAG_DELETED_NODE", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DELETED_PATH, "INDEX_TREE_NODE_FLAG_DELETED_PATH", bAppendComma);
	bAppendComma |= psz->AppendFlag(msFlags, INDEX_TREE_NODE_FLAG_DIRTY_NODE, "INDEX_TREE_NODE_FLAG_DIRTY_NODE", bAppendComma);
	return psz->Text();
}

