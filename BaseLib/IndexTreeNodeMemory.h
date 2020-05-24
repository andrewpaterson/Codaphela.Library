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

	void					Clear(unsigned char uiIndex);
	BOOL					ClearAndUncontain(unsigned char uiIndex);

	CIndexTreeNodeMemory*	GetNode(int i);
	CIndexTreeNodeMemory**	GetNodes(void);
	int						NumValidIndexes(void);
	CIndexTreeNodeMemory*	GetParent(void);

	void					Contain(unsigned char uiIndex);
	BOOL					Uncontain(unsigned char uiIndex);

	void					RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode);

	unsigned char			FindPrevLastIndex(void);
	unsigned char			FindNextFirstIndex(void);
	unsigned char			FindIndex(CIndexTreeNodeMemory* pcChild);

	void					SetChildrensParent(void);

	BOOL					ValidateNodesEmpty(void);
	void					Print(CChars* psz, BOOL bHex);
	void					Dump(void);
};


#endif // __INDEX_TREE_NODE_MEMORY_H__

