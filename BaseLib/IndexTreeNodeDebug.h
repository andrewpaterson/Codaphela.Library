#ifndef __INDEX_TREE_NODE_DEBUG_H__
#define __INDEX_TREE_NODE_DEBUG_H__
#include "IndexTreeNode.h"


class CIndexTreeNodeDebug
{
protected:
	CChars	mszBadKey;
	CChars	mszBadNode;

public:
	void Init(void);
	void Kill(void);

	void GenerateBad(CIndexTreeNode* pcCurrent, CArrayChar* acKey);
	char* GetBadKey(void);
	char* GetBadNode(void);

protected:
	void GenerateBadNode(CIndexTreeNode* pcCurrent, BOOL bHex);
	BOOL GenerateBadKey(CArrayChar* acKey);
};


#endif // !__INDEX_TREE_NODE_DEBUG_H__

