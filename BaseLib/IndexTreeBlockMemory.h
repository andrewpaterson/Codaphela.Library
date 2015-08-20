#ifndef __INDEX_TREE_BLOCK_MEMORY_H__
#define __INDEX_TREE_BLOCK_MEMORY_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
#include "IndexTreeNodeMemory.h"
#include "MemoryCache.h"


struct SIndexTreeIterator
{
	CIndexTreeNodeMemory*	pcNode;
	int						iIndex;
};


class CIndexTreeBlockMemory
{
protected:
	CIndexTreeNodeMemory*	mpcRoot;
	int						miSize;
	CMallocator*			mpcMalloc;
	int						miLargestKeySize;

public:
	void					Init(void);
	void					Init(CMallocator* pcMalloc);
	void					Kill(void);
	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	void*					Get(char* pszKey);
	void*					Get(void* pvKey, int iKeySize);

	int						NumElements(void);
	int						GetLargestKeySize(void);
	int						GetKey(void* pvKey, void* pvData, BOOL zeroTerminate = FALSE);

	BOOL					StartIteration(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);

	BOOL					Put(char* pszKey, void* pvObject, unsigned char uiObjectSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize);
	void*					Put(void* pvKey, int iKeySize, unsigned char uiObjectSize);
	BOOL					PutPtr(char* pszKey, void* pvPointer);
	BOOL					PutPtr(void* pvKey, int iKeySize, void* pvPointer);

	BOOL					Remove(char* pszKey);
	BOOL					Remove(void* pvKey, int iKeySize);

	BOOL					HasKey(char* pszKey);
	BOOL					HasKey(void* pvKey, int iKeySize);

	void					FindAll(CArrayVoidPtr* papvElements);

	CIndexTreeNodeMemory*	GetIndexNode(void* pvKey, int iKeySize);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);

	BOOL					Write(CFileWriter* pcFileWriter);
	BOOL					Read(CFileReader* pcFileReader);

protected:
	void*					Malloc(size_t tSize);
	void*					Realloc(void* pv, size_t tSize);
	void					Free(void* pv);

	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent);
	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, unsigned char uiDataSize);
	void					FreeNode(CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c);
	void					RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements);
	BOOL					ValidateSize(void);
	int						RecurseSize(CIndexTreeNodeMemory* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeMemory* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	void					RemapChildParents(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);
	size_t					SizeofNode(void);
	size_t					SizeofNodePtr(void);

	BOOL					StepNext(SIndexTreeIterator* psIterator);
};


#endif // __INDEX_TREE_BLOCK_H__

