#ifndef __INDEX_TREE_NODE_MEMORY_H__
#define __INDEX_TREE_NODE_MEMORY_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeMemory : public CIndexTreeNode
{
public:
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiIndexInParent);
	void					Init(CIndexTree* pcIndexTree, CIndexTreeNode* pcParent, unsigned char uiIndexInParent);

	CIndexTreeNodeMemory*	Get(unsigned char uiIndex);
	void					Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode);

	void					ClearIndex(unsigned char uiIndex);
	bool					ClearAndUncontain(unsigned char uiIndex);

	CIndexTreeNodeMemory*	GetNode(int i);
	CIndexTreeNodeMemory**	GetNodes(void);
	int						NumValidIndexes(void);
	CIndexTreeNodeMemory*	GetParent(void);

	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					RemapDataLinks(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);
	void					Contain(unsigned char uiIndex);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNodeMemory* pcChild);

	void					SetChildrensParent(void);

	bool					ValidateNodesEmpty(void);
	void					Print(CChars* psz, bool bHex);
	void					Dump(void);

	bool					Uncontain(unsigned char uiIndex);
};


#endif // !__INDEX_TREE_NODE_MEMORY_H__

