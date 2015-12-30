#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeMemory : public CIndexTreeNode
{
public:
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeMemory* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	void					Init(CIndexTreeBlock* pcIndexTree, CIndexTreeNodeMemory* pcParent);

	CIndexTreeNodeMemory*	Get(unsigned char uiIndex);
	void					Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

