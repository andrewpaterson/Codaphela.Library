#ifndef __INDEX_TREE_BLOCK_MEMORY_H__
#define __INDEX_TREE_BLOCK_MEMORY_H__
#include "IndexTreeNodeMemory.h"
#include "IndexTree.h"

struct SIndexTreeIterator
{
	CIndexTreeNodeMemory*	pcNode;
	int						iIndex;
};


class CIndexTreeMemory : public CIndexTree
{
protected:
	CIndexTreeNodeMemory*	mpcRoot;
	int						miSize;
public:
	void					Init(void);
	void					Init(CMallocator* pcMalloc);
	void					FakeInit(void);
	void					Kill(void);
	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	void*					Get(char* pszKey);
	void*					Get(void* pvKey, int iKeySize);

	int						NumElements(void);
	int						GetLargestKeySize(void);
	int						GetKey(void* pvKey, void* pvData, BOOL zeroTerminate = FALSE);

	BOOL					StartIteration(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeIterator* psIterator, void** pvData, int* piDataSize);

	void*					Put(char* pszKey, void* pvObject, unsigned char uiObjectSize);
	void*					Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize);
	void*					Put(void* pvKey, int iKeySize, unsigned char uiObjectSize);

	BOOL					Remove(char* pszKey);
	BOOL					Remove(void* pvKey, int iKeySize);

	BOOL					HasKey(char* pszKey);
	BOOL					HasKey(void* pvKey, int iKeySize);

	void					FindAll(CArrayVoidPtr* papvElements);

	BOOL					PutPtr(char* pszKey, void* pvPointer);
	BOOL					PutPtr(void* pvKey, int iKeySize, void* pvPointer);
	BOOL					PutLong(char* pszKey, int64 lliIndex);
	BOOL					PutLong(void* pvKey, int iKeySize, int64 lliIndex);

	CIndexTreeNodeMemory*	GetIndexNode(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	GetRoot(void);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);

	BOOL					Write(CFileWriter* pcFileWriter);
	BOOL					Read(CFileReader* pcFileReader);

	BOOL					ValidateIndexTree(void);
	BOOL					ValidateSize(void);

protected:
	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent);

	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, unsigned char uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, unsigned char uiOriginalSize);
	CIndexTreeNodeMemory*	ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);


	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c);
	void					RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements);
	int						RecurseSize(CIndexTreeNodeMemory* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeMemory* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);

	BOOL					StepNext(SIndexTreeIterator* psIterator);
};


#endif // __INDEX_TREE_BLOCK_MEMORY_H__

