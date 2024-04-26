#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Define.h"
#include "DataTypes.h"
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
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint16 uiDataSize, int iClearValue, uint8 uiIndexInParent);
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiIndexInParent);

	void				SetData(void* pvObject, uint16 uiSize);

	uint16				GetDataSize(void);
	CIndexTreeDataNode* GetNodeData(void);
	void*				GetDataPtr(void);
	bool				HasData(void);
	void				SetData(bool bHasData);

	void*				GetNodesMemory(void);

	CIndexTreeNode*		GetParent(void);
	uint8				GetIndexInParent(void);
	uint8				GetFlags(void);
	uint8				GetTransientFlags(void);
	CIndexTree*			GetIndexTree(void);

	int					NumIndexes(void);
	int					NumIndexes(uint8 uiFirstIndex, uint8 uiLastIndex);
	int					GetAdditionalIndexes(uint8 uiIndex);

	bool				IsEmpty(void);
	bool				IsDirty(void);
	bool				IsPathDirty(void);
	bool				IsDeleted(void);
	bool				IsPathDeleted(void);
	bool				IsMagic(void);

	bool				HasFlags(uint8 sFlags);
	void				ClearFlags(uint8 sFlags);
	uint8				GetFirstIndex(void);
	uint8				GetLastIndex(void);
	bool				HasNodes(void);
	uint8				NumAllocatedNodes(void);
	bool				ContainsIndex(uint8 uiIndex);
	void				SetNodesEmpty(bool bEmpty);
	void				SetDirtyNode(bool bDirty);
	void				SetDeletedNode(bool bDirty);
	void				SetDirtyPath(bool bDirty);
	void				SetDeletedPath(bool bDirty);

	void				Contain(uint8 uiIndex, int iClearValue);
	void				ClearOnlyNode(uint8 uiIndex, int iClearValue);
	void				MoveNodesLeft(uint8 uiNextFirstIndex);
	void				MoveNodesRight(uint8 uiNewFirstIndex, int iClearValue);
	void				ClearLastNodes(uint8 uiNewLastIndex, int iClearValue);

	size_t				SizeofNodeAndData(void);  //Rename to SizeofNode.
	size_t				SizeofNode(void);  //Remove this.
	size_t				SizeofNodePtr(void);

	void				Print(CChars* psz, bool bHex);
	char*				GetFlagsString(CChars* psz);

	size_t				CalculateRequiredNodeSizeForIndex(uint8 uiIndex);
	size_t				CalculateRequiredNodeSizeForData(uint16 uiDataSize);
	size_t				CalculateRequiredNodeSizeForCurrent(void);

protected:
	void				ChangeDataSize(uint16 uiSize);
};


#endif // __INDEX_TREE_NODE_H__

