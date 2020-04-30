#ifndef __INDEX_TREE_RECURSOR_H__
#define __INDEX_TREE_RECURSOR_H__
#include "ArrayChar.h"
#include "IndexTreeNode.h"


class CIndexTreeRecursor
{
private:
	CArrayChar			macKey;
	CIndexTreeNode*		mpcCurrent;

	CChars				mszBadKey;
	CChars				mszBadNode;

public:
	void				Init(CIndexTreeNode* pcRoot);
	void				Kill(void);

	void				Push(CIndexTreeNode* pcChild, char c);
	void				Pop(void);

	void				GenerateBad(void);

	CIndexTreeNode*		GetNode(void);
	char*				GetBadKey(void);
	char*				GetBadNode(void);

	void				GetKey(char* pc, int* piKeySize);

protected:
	BOOL				GenerateBadKey(void);
	void				GenerateBadNode(BOOL bHex);
};


#endif // __INDEX_TREE_RECURSOR_H__


