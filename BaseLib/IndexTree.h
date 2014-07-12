#ifndef __INDEX_TREE_H__
#define __INDEX_TREE_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
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

	void*				Get(char* pszKey);
	void*				Get(void* pvKey, int iKeySize);

	int					NumElements(void);

	BOOL				Put(void* pvObject, unsigned char uiObjectSize, char* pszKey);
	BOOL				Put(void* pvObject, unsigned char uiObjectSize, void* pvKey, int iKeySize);
	BOOL				PutPtr(void* pvPointer, char* pszKey);
	BOOL				PutPtr(void* pvPointer, void* pvKey, int iKeySize);

	BOOL				Remove(char* pszKey);
	BOOL				Remove(void* pvKey, int iKeySize);

	BOOL				HasKey(char* pszKey);
	BOOL				HasKey(void* pvKey, int iKeySize);

	void				FindAll(CArrayVoidPtr* papvElements);

	int					GetModifications(void);
	CIndexTreeNode*		GetIndexNode(void* pvKey, int iKeySize);
	int					CountAllocatedNodes(void);
	int					RecurseSize(void);

protected:
	void*				Malloc(size_t tSize);
	void*				Realloc(void* pv, size_t tSize);
	void				Free(void* pv);

	CIndexTreeNode*		AllocateRoot(void);
	CIndexTreeNode*		AllocateNode(CIndexTreeNode* pcParent);
	CIndexTreeNode*		ReallocateNodeForIndex(CIndexTreeNode* pcNode, unsigned char uiIndex);
	CIndexTreeNode*		ReallocateNodeForData(CIndexTreeNode* pcNode, unsigned char uiDataSize);
	void				FreeNode(CIndexTreeNode* pcNode);

	CIndexTreeNode*		SetOldWithCurrent(CIndexTreeNode* pcParent, unsigned char c);
	void				RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements);
	BOOL				ValidateSize(void);
	int					RecurseSize(CIndexTreeNode* pcNode);
	int					RecurseCountAllocatedNodes(CIndexTreeNode* pcNode);
	int					CountListSize(void);
	int					RecurseCountListSize(CIndexTreeNode* pcNode);
};


#endif // __INDEX_TREE_H__

