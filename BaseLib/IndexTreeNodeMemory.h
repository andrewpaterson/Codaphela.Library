#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeMemory : public CIndexTreeNode
{
public:
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNode* pcParent);

	CIndexTreeNodeMemory*	Get(unsigned char uiIndex);
	void					Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode);

	BOOL					Clear(unsigned char uiIndex);

	CIndexTreeNodeMemory*	GetNode(int i);
	CIndexTreeNodeMemory**	GetNodes(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNodeMemory* pcChild);

	void					SetChildsParent(void);

	BOOL					ValidateNodesEmpty(void);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

