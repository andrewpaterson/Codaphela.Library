#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "Bool.h"
#include "DataTypes.h"


class CIndexTreeNodeMemory
{
protected:
	CIndexTreeNodeMemory*	mpcParent;
	unsigned char			muiFirstIndex;
	unsigned char			muiLastIndex;  // Inclusive (because 255 is a legitimate last index and 256 can't be represented)
	unsigned char			muiDataSize;   // Size of object "pointed" to by this node.  The object is small: usually a OIndex, a pointer or a CFileId.
	unsigned char			mbNodesEmpty;

public:
	void					Init(CIndexTreeNodeMemory* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTreeNodeMemory* pcParent);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);
	void					SizeObject(unsigned char uiSize);

	CIndexTreeNodeMemory*	Get(unsigned char uiIndex);
	void*					GetObjectPtr(void);
	unsigned char			GetObjectSize(void);

	void					Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode);
	BOOL					Clear(unsigned char uiIndex);
	BOOL					SetObject(void* pvObject, unsigned char uiSize);
	void					ClearObject(void);

	BOOL					IsEmpty(void);
	unsigned char			GetFirstIndex(void);
	unsigned char			GetLastIndex(void);
	BOOL					HasNodes(void);
	unsigned char			NumNodes(void);
	BOOL					ContainsIndex(unsigned char uiIndex);
	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNodeMemory* pcChild);

	int						GetNumIndexes(void);
	CIndexTreeNodeMemory*	GetNode(int i);
	CIndexTreeNodeMemory*	GetParent(void);
	CIndexTreeNodeMemory**	GetNodes(void);

	int						GetAdditionalIndexes(unsigned char uiIndex);
	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					SetChildsParent(void);
	int						GetNumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex);

	size_t					CalculateRequiredNodeSizeForIndex(unsigned char uiIndex);
	size_t					CalculateRequiredNodeSizeForEmpty(void);
	size_t					CalculateRequiredNodeSizeForData(unsigned char uiDataSize);
	size_t					CalculateRequiredNodeSizeForCurrent(void);

	size_t					SizeofNode(void);
	size_t					SizeofNodePtr(void);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

