#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Define.h"
#include "DataTypes.h"
#include "Chars.h"
#include "IndexTreeDataNode.h"

//
//  Bytes 0     [muiFirstIndex ..... muiLastIndex]       255
//
//  mpcIndexTree, mpcParent, muiFirstIndex, muiLastIndex, muiDataSize, mbNodesEmpty, Object[muiDataSize], Nodes[muiLastIndex-muiFirstIndex+1]


#define	INDEX_TREE_NODE_FLAG_DIRTY_NODE		0x02
#define	INDEX_TREE_NODE_FLAG_NODES_EMPTY	0x04
#define	INDEX_TREE_NODE_FLAG_DELETED_NODE	0x08
#define	INDEX_TREE_NODE_FLAG_DELETED_PATH	0x10
#define	INDEX_TREE_NODE_FLAG_DIRTY_PATH		0x20
#define	INDEX_TREE_NODE_FLAG_DATA			0x40

#define INDEX_TREE_NODE_TRANSIENT_FLAGS			((uint8)(INDEX_TREE_NODE_FLAG_DIRTY_PATH | INDEX_TREE_NODE_FLAG_DELETED_PATH | INDEX_TREE_NODE_FLAG_DELETED_NODE | INDEX_TREE_NODE_FLAG_DIRTY_NODE))
#define INDEX_TREE_NODE_TRANSIENT_FLAGS_MASK		(~INDEX_TREE_NODE_TRANSIENT_FLAGS)

#define INDEX_TREE_NODE_MAGIC	0x5A37


//sizeof(CIndexTreeNode) is 16 bytes but only 14 bytes are used.
class CIndexTree;
class CIndexTreeNode
{
protected:
	CIndexTree*			mpcIndexTree;
	CIndexTreeNode*		mpcParent;

	uint16				muiMagic;  //Always set to INDEX_TREE_NODE_MAGIC

	uint8				muiFirstIndex;
	uint8				muiLastIndex;  // Inclusive (because 255 is a legitimate last index and 256 can't be represented)

	uint8				muiIndexInParent;
	uint8				msFlags;

public:
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiFirstIndex, size uiLastIndex, size uiDataSize, size iClearValue, size uiIndexInParent);
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiIndexInParent);

	void				SetData(void* pvObject, size uiSize);

	size				GetDataSize(void);
	CIndexTreeDataNode* GetNodeData(void);
	void*				GetDataPtr(void);
	bool				HasData(void);
	void				SetData(bool bHasData);

	void*				GetNodesMemory(void);

	CIndexTreeNode*		GetParent(void);
	size				GetIndexInParent(void);
	size				GetFlags(void);
	size				GetTransientFlags(void);
	CIndexTree*			GetIndexTree(void);

	size				NumIndexes(void);
	size				NumIndexes(size uiFirstIndex, size uiLastIndex);
	size				GetAdditionalIndexes(size uiIndex);

	bool				IsEmpty(void);
	bool				IsDirty(void);
	bool				IsPathDirty(void);
	bool				IsDeleted(void);
	bool				IsPathDeleted(void);
	bool				IsMagic(void);

	bool				HasFlags(size sFlags);
	void				ClearFlags(size sFlags);
	size				GetFirstIndex(void);
	size				GetLastIndex(void);
	bool				HasNodes(void);
	size				NumAllocatedNodes(void);
	bool				ContainsIndex(size uiIndex);
	void				SetNodesEmpty(bool bEmpty);
	void				SetDirtyNode(bool bDirty);
	void				SetDeletedNode(bool bDirty);
	void				SetDirtyPath(bool bDirty);
	void				SetDeletedPath(bool bDirty);

	void				Contain(size uiIndex, size iClearValue);
	void				ClearOnlyNode(size uiIndex, size iClearValue);
	void				MoveNodesLeft(size uiNextFirstIndex);
	void				MoveNodesRight(size uiNewFirstIndex, size iClearValue);
	void				ClearLastNodes(size uiNewLastIndex, size iClearValue);

	size				SizeofNodeAndData(void);  //Rename to SizeofNode.
	size				SizeofNode(void);  //Remove this.
	size				SizeofNodePtr(void);

	void				Print(CChars* psz, bool bHex);
	char*				GetFlagsString(CChars* psz);

	size				CalculateRequiredNodeSizeForIndex(size uiIndex);
	size				CalculateRequiredNodeSizeForData(size uiDataSize);
	size				CalculateRequiredNodeSizeForCurrent(void);

protected:
	void				ChangeDataSize(size uiSize);
};


#endif // __INDEX_TREE_NODE_H__

