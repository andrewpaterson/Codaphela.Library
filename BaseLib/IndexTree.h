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
	CIndexTreeNode*		ReallocateNodeForIndex(CIndexTreeNode* pcNode, unsigned char uiIndex);
	CIndexTreeNode*		ReallocateNodeForData(CIndexTreeNode* pcNode, unsigned char uiDataSize);
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

	int					NumElements(void);
	BOOL				Contains(char* pszKey);
	BOOL				Add(char* pszKey, void* pvObject);

protected:
	void*				Malloc(size_t tSize);
	void*				Realloc(void* pv, size_t tSize);
	void				Free(void* pv);
};


#endif // __INDEX_TREE_H__

