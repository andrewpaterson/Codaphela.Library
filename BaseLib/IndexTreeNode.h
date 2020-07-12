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

#define INDEX_TREE_NODE_TRANSIENT_FLAGS			((unsigned char)(INDEX_TREE_NODE_FLAG_DIRTY_PATH | INDEX_TREE_NODE_FLAG_DELETED_PATH | INDEX_TREE_NODE_FLAG_DELETED_NODE | INDEX_TREE_NODE_FLAG_DIRTY_NODE))
#define INDEX_TREE_NODE_TRANSIENT_FLAGS_MASK		(~INDEX_TREE_NODE_TRANSIENT_FLAGS)

#define INDEX_TREE_NODE_MAGIC	0x5A37


//sizeof(CIndexTreeNode) is 16 bytes but only 14 bytes are used.
class CIndexTree;
class CIndexTreeNode
{
protected:
	CIndexTree*			mpcIndexTree;
	CIndexTreeNode*		mpcParent;

	unsigned short		muiMagic;  //Always set to INDEX_TREE_NODE_MAGIC

	unsigned char		muiFirstIndex;
	unsigned char		muiLastIndex;  // Inclusive (because 255 is a legitimate last index and 256 can't be represented)

	unsigned char		muiIndexInParent;
	unsigned char		msFlags;

public:
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, int iClearValue, unsigned char uiIndexInParent);
	void				Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiIndexInParent);

	void				SetData(void* pvObject, unsigned short uiSize);

	unsigned short		GetDataSize(void);
	CIndexTreeDataNode* GetNodeData(void);
	void*				GetDataPtr(void);
	BOOL				HasData(void);
	void				SetData(BOOL bHasData);

	void*				GetNodesMemory(void);

	CIndexTreeNode*		GetParent(void);
	unsigned char		GetIndexInParent(void);
	unsigned char		GetFlags(void);
	unsigned char		GetTransientFlags(void);

	int					NumIndexes(void);
	int					NumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex);
	int					GetAdditionalIndexes(unsigned char uiIndex);

	BOOL				IsEmpty(void);
	BOOL				IsDirty(void);
	BOOL				IsPathDirty(void);
	BOOL				IsDeleted(void);
	BOOL				IsPathDeleted(void);
	BOOL				IsMagic(void);

	BOOL				HasFlags(unsigned char sFlags);
	void				ClearFlags(unsigned char sFlags);
	unsigned char		GetFirstIndex(void);
	unsigned char		GetLastIndex(void);
	BOOL				HasNodes(void);
	unsigned char		NumAllocatedNodes(void);
	BOOL				ContainsIndex(unsigned char uiIndex);
	void				SetNodesEmpty(BOOL bEmpty);
	void				SetDirtyNode(BOOL bDirty);
	void				SetDeletedNode(BOOL bDirty);
	void				SetDirtyPath(BOOL bDirty);
	void				SetDeletedPath(BOOL bDirty);

	void				Contain(unsigned char uiIndex, int iClearValue);
	void				ClearOnlyNode(unsigned char uiIndex, int iClearValue);
	void				MoveNodesLeft(unsigned char uiNextFirstIndex);
	void				MoveNodesRight(unsigned char uiNewFirstIndex, int iClearValue);
	void				ClearLastNodes(unsigned char uiNewLastIndex, int iClearValue);

	size_t				SizeofNodeAndData(void);  //Rename to SizeofNode.
	size_t				SizeofNode(void);  //Remove this.
	size_t				SizeofNodePtr(void);

	void				Print(CChars* psz, BOOL bHex);
	char*				GetFlagsString(CChars* psz);

	size_t				CalculateRequiredNodeSizeForIndex(unsigned char uiIndex);
	size_t				CalculateRequiredNodeSizeForData(unsigned short uiDataSize);
	size_t				CalculateRequiredNodeSizeForCurrent(void);

protected:
	void				ChangeDataSize(unsigned short uiSize);
};


#endif // __INDEX_TREE_NODE_H__

