#ifndef __INDEX_TREE_BLOCK_MEMORY_H__
#define __INDEX_TREE_BLOCK_MEMORY_H__
#include "IndexTreeNodeMemory.h"
#include "IndexTreeBlock.h"

struct SIndexTreeIterator
{
	CIndexTreeNode*	pcNode;
	int				iIndex;
};


class CIndexTreeBlockMemory : public CIndexTreeBlock
{
protected:
	CIndexTreeNodeMemory*	mpcRoot;

public:
	void					Init(void);
	void					Init(CMallocator* pcMalloc);
	void					Kill(void);
	void					RecurseKill(CIndexTreeNode* pcNode);

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
	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent);
	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForData(CIndexTreeNodeMemory* pcNode, unsigned char uiDataSize);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char c);
	void					RecurseFindAll(CIndexTreeNode* pcNode, CArrayVoidPtr* papvElements);
	BOOL					ValidateSize(void);
	int						RecurseSize(CIndexTreeNode* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNode* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);

	BOOL					StepNext(SIndexTreeIterator* psIterator);
};


#endif // __INDEX_TREE_BLOCK_MEMORY_H__

