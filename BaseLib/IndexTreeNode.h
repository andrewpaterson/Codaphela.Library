#ifndef __INDEX_TREE_NODE_H__
#define __INDEX_TREE_NODE_H__
#include "Bool.h"
#include "DataTypes.h"


//
//  Bytes 0     [muiFirstIndex ..... muiLastIndex]       255  
//  


class CIndexTreeBlock;
class CIndexTreeNode
{
public:
	CIndexTreeBlock*		mpcIndexTree;
	CIndexTreeNode*			mpcParent;
	unsigned char			muiFirstIndex;
	unsigned char			muiLastIndex;  // Inclusive (because 255 is a legitimate last index and 256 can't be represented)
	unsigned char			muiDataSize;   // Size of object "pointed" to by this node.  The object is small: usually a OIndex, a pointer or a CFileId.
	unsigned char			mbNodesEmpty;

public:
	CIndexTreeNode*			Get(unsigned char uiIndex);

	BOOL					Clear(unsigned char uiIndex);

	void					SizeObject(unsigned char uiSize);
	BOOL					SetObject(void* pvObject, unsigned char uiSize);
	void					ClearObject(void);

	unsigned char			GetObjectSize(void);
	void*					GetObjectPtr(void);

	void					RemapChildNodes(CIndexTreeNode* pcOldNode, CIndexTreeNode* pcNewNode);

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
	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNode* pcChild);

	CIndexTreeNode*			GetNode(int i);
	CIndexTreeNode**		GetNodes(void);

	void					SetChildsParent(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	size_t					CalculateRequiredNodeSizeForIndex(unsigned char uiIndex);
	size_t					CalculateRequiredNodeSizeForEmpty(void);
	size_t					CalculateRequiredNodeSizeForData(unsigned char uiDataSize);
	size_t					CalculateRequiredNodeSizeForCurrent(void);

	size_t					SizeofNode(void);
	size_t					SizeofNodePtr(void);
};


#endif // __INDEX_TREE_NODE_H__

