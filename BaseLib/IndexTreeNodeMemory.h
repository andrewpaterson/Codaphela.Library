#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeMemory : public CIndexTreeNode
{
public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint8 uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, uint8 uiIndexInParent);

	CIndexTreeNodeMemory*	Get(uint8 uiIndex);
	void					Set(uint8 uiIndex, CIndexTreeNodeMemory* pcNode);

	void					ClearIndex(uint8 uiIndex);
	bool					ClearAndUncontain(uint8 uiIndex);

	CIndexTreeNodeMemory*	GetNode(int i);
	CIndexTreeNodeMemory**	GetNodes(void);
	int						NumValidIndexes(void);
	CIndexTreeNodeMemory*	GetParent(void);

	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					RemapDataLinks(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					Contain(uint8 uiIndex);

	uint8					FindPrevLastIndex(void);
	uint8					FindNextFirstIndex(void);
	uint8					FindIndex(CIndexTreeNodeMemory* pcChild);

	void					SetChildrensParent(void);

	bool					ValidateNodesEmpty(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

	bool					Uncontain(uint8 uiIndex);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

