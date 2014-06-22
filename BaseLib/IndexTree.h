#ifndef __INDEX_TREE_H__
#define __INDEX_TREE_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayEmbedded.h"
#include "IndexTreeNode.h"
#include "MemoryCache.h"


class CIndexTree
{
protected:
	CIndexTreeNode*		mpcRoot;
	int					miSize;
	int					miModifications;

public:
	void				Init(void);
	void				Kill(void);
	void				RecurseKill(CIndexTreeNode* pcNode);
	CIndexTreeNode*		AllocateRoot(void);
	CIndexTreeNode*		AllocateNode(CIndexTreeNode* pcParent);
	CIndexTreeNode*		ReallocateNode(CIndexTreeNode* pcNode, unsigned char uiIndex);
	void				FreeNode(CIndexTreeNode* pcNode);
	CIndexTreeNode*		GetIndexNode(char* pszKey);
	void*				Get(char* pszKey);
	BOOL				Put(void* pvObject, char* pszKey);
	void*				Remove(char* pszKey);
	CIndexTreeNode*		SetOldWithCurrent(CIndexTreeNode* pcParent, unsigned char c);
	void				FindAll(CArrayVoidPtr* papvElements);
	void				RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements);
	int					GetSize(void);
	BOOL				ValidateSize(void);
	int					RecurseSize(void);
	int					RecurseSize(CIndexTreeNode* pcNode);
	BOOL				HasKey(char* pszKey);
	int					CountAllocatedNodes(void);
	int					RecurseCountAllocatedNodes(CIndexTreeNode* pcNode);
	int					CountListSize(void);
	int					RecurseCountListSize(CIndexTreeNode* pcNode);
	int					GetModifications(void);
};


#endif // __INDEX_TREE_H__

