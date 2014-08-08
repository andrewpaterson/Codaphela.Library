#ifndef __INDEX_TREE_BLOCK_H__
#define __INDEX_TREE_BLOCK_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
#include "IndexTreeNode.h"
#include "MemoryCache.h"


struct SIndexTreeIterator
{
	CIndexTreeNode*		pcNode;
	int					iIndex;
};


class CIndexTreeBlock
{
protected:
	CIndexTreeNode*		mpcRoot;
	int					miSize;
	CMallocator*		mpcMalloc;
	int					miLargestKeySize;

public:
	void				Init(void);
	void				Init(CMallocator* pcMalloc);
	void				Kill(void);
	void				RecurseKill(CIndexTreeNode* pcNode);

	void*				Get(char* pszKey);
	void*				Get(void* pvKey, int iKeySize);

	int					NumElements(void);
	int					GetLargestKeySize(void);
	int					GetKey(void* pvKey, void* pvData, BOOL zeroTerminate = FALSE);

	BOOL				StartIteration(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);
	BOOL				Iterate(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);

	BOOL				Put(char* pszKey, void* pvObject, unsigned char uiObjectSize);
	BOOL				Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize);
	void*				Put(void* pvKey, int iKeySize, unsigned char uiObjectSize);
	BOOL				PutPtr(char* pszKey, void* pvPointer);
	BOOL				PutPtr(void* pvKey, int iKeySize, void* pvPointer);

	BOOL				Remove(char* pszKey);
	BOOL				Remove(void* pvKey, int iKeySize);

	BOOL				HasKey(char* pszKey);
	BOOL				HasKey(void* pvKey, int iKeySize);

	void				FindAll(CArrayVoidPtr* papvElements);

	CIndexTreeNode*		GetIndexNode(void* pvKey, int iKeySize);
	int					CountAllocatedNodes(void);
	int					RecurseSize(void);

	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader);

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
	void				RemapChildParents(CIndexTreeNode* pcOldNode, CIndexTreeNode* pcNode);
	CIndexTreeNode*		GetNodeForData(void* pvData);
	size_t				SizeofNode(void);
	size_t				SizeofNodePtr(void);

	BOOL				StepNext(SIndexTreeIterator* psIterator);
};


#endif // __INDEX_TREE_BLOCK_H__

