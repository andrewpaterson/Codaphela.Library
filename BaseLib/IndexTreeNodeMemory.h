#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeMemory : public CIndexTreeNode
{
public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiFirstIndex, size uiLastIndex, size uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, size uiIndexInParent);

	CIndexTreeNodeMemory*	Get(size uiIndex);
	void					Set(size uiIndex, CIndexTreeNodeMemory* pcNode);

	void					ClearIndex(size uiIndex);
	bool					ClearAndUncontain(size uiIndex);

	CIndexTreeNodeMemory*	GetNode(size i);
	CIndexTreeNodeMemory**	GetNodes(void);
	size					NumValidIndexes(void);
	CIndexTreeNodeMemory*	GetParent(void);

	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					RemapvDataLinks(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					Contain(size uiIndex);

	size					FindPrevLastIndex(void);
	size					FindNextFirstIndex(void);
	size					FindIndex(CIndexTreeNodeMemory* pcChild);

	void					SetChildrensParent(void);

	bool					ValidateNodesEmpty(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

	bool					Uncontain(size uiIndex);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

