#ifndef __INDEX_TREE_MEMORY_H__
#define __INDEX_TREE_MEMORY_H__
#include "IndexTreeNodeMemory.h"
#include "IndexTreeRecursor.h"
#include "IndexTree.h"


struct SIndexTreeMemoryIterator
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
	void					Init(EIndexKeyReverse eKeyReverse = IKR_No);
	void					Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse);
	void					Kill(void);

	void*					Get(void* pvKey, int iKeySize, unsigned short* puiDataSize);
	void*					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	void					FindAll(CArrayVoidPtr* papvElements);
	int						GetKey(void* pvKey, void* pvData, BOOL zeroTerminate = FALSE);

	int						NumElements(void);

	BOOL					StartIteration(SIndexTreeMemoryIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeMemoryIterator* psIterator, void** pvData, int* piDataSize);

	CIndexTreeNodeMemory*	GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	GetRoot(void);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);
	int						NumAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);

	BOOL					Write(CFileWriter* pcFileWriter);
	BOOL					Read(CFileReader* pcFileReader, EIndexKeyReverse eKeyReverse);

	void					FakeInit(EIndexKeyReverse eKeyReverse);
	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	BOOL					ValidateIndexTree(void);
	BOOL					ValidateSize(void);

protected:
	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent, unsigned char uiIndexInParent);
	BOOL					Remove(CIndexTreeNodeMemory* pcCurrent);

	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, unsigned short uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, unsigned short uiOriginalSize);
	CIndexTreeNodeMemory*	ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c);
	void					RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements);
	int						RecurseSize(CIndexTreeNodeMemory* pcNode);
	int						RecurseNumNodes(CIndexTreeNodeMemory* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeMemory* pcNode);
	BOOL					ValidateLimits(void);
	BOOL					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);
	BOOL					ValidateParentIndex(void);
	BOOL					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor);

	BOOL					StepNext(SIndexTreeMemoryIterator* psIterator);
};


#endif // __INDEX_TREE_BLOCK_H__

