#ifndef __INDEX_TREE_RECURSOR_H__
#define __INDEX_TREE_RECURSOR_H__
#include "ArrayChar.h"
#include "IndexTreeNodeDebug.h"


//The key buffer starts at this size and only ever grows, so recursing up and down the tree does not reallocate it.
#define INDEX_TREE_RECURSOR_KEY_CHUNK_SIZE	64


class CIndexTreeRecursor : public CIndexTreeNodeDebug
{
protected:
	CArrayChar			macKey;
	CIndexTreeNode*		mpcCurrent;

public:
	void				Init(CIndexTreeNode* pcRoot);
	void				Kill(void);

	void				Push(CIndexTreeNode* pcChild, uint8 c);
	void				Pop(void);

	void				GenerateBad(void);

	CIndexTreeNode*		GetNode(void);

	void				GetKey(void* pc, size* piKeySize);
	size				GetKeySize(void);
};


#endif // __INDEX_TREE_RECURSOR_H__

