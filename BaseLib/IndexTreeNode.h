#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Bool.h"
#include "DataTypes.h"


//
//  Bytes 0     [muiFirstIndex ..... muiLastIndex]       255
//
//  mpcIndexTree, mpcParent, muiFirstIndex, muiLastIndex, muiDataSize, mbNodesEmpty, Object[muiDataSize], Nodes[muiLastIndex-muiFirstIndex+1]


class CIndexTree;
class CIndexTreeNode
{
public:
	CIndexTree*				mpcIndexTree;
	CIndexTreeNode*			mpcParent;
	unsigned char			muiFirstIndex;
	unsigned char			muiLastIndex;  // Inclusive (because 255 is a legitimate last index and 256 can't be represented)
	unsigned char			muiDataSize;   // Size of object "pointed" to by this node.  The object is small: usually a OIndex, a pointer or a CFileId.
	unsigned char			mbNodesEmpty;

public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize, int iClearValue);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent);

	void					ChangeDataSize(unsigned char uiSize);
	BOOL					SetObject(void* pvObject, unsigned char uiSize);
	void					ClearObject(void);

	unsigned char			GetObjectSize(void);
	void*					GetObjectPtr(void);
	BOOL					HasObject(void);

	void*					GetNodesMemory(void);

	CIndexTreeNode*			GetParent(void);

	int						GetNumIndexes(void);
	int						GetNumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex);
	int						GetAdditionalIndexes(unsigned char uiIndex);

	BOOL					IsEmpty(void);
	unsigned char			GetFirstIndex(void);
	unsigned char			GetLastIndex(void);
	BOOL					HasNodes(void);
	unsigned char			NumNodes(void);
	BOOL					ContainsIndex(unsigned char uiIndex);

	void					ClearOnlyNode(unsigned char uiIndex, int iClearValue);
	void					MoveNodesLeft(unsigned char uiNextFirstIndex);
	void					MoveNodesRight(unsigned char uiNewFirstIndex, int iClearValue);
	void					ClearLastNodes(unsigned char uiNewLastIndex, int iClearValue);

	size_t					CalculateRequiredNodeSizeForIndex(unsigned char uiIndex);
	size_t					CalculateRequiredNodeSizeForEmpty(void);
	size_t					CalculateRequiredNodeSizeForData(unsigned char uiDataSize);
	size_t					CalculateRequiredNodeSizeForCurrent(void);

	size_t					SizeofNode(void);
	size_t					SizeofNodePtr(void);
};


#endif // __INDEX_TREE_NODE_H__

