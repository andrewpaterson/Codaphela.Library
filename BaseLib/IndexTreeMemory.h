#ifndef __INDEX_TREE_MEMORY_H__
#define __INDEX_TREE_MEMORY_H__
#include "PrimitiveTypes.h"
#include "IndexTreeNodeMemory.h"
#include "IndexTreeRecursor.h"
#include "IndexTreeMemoryConfig.h"
#include "IndexTree.h"


struct SIndexTreeMemoryUnsafeIterator
{
	CIndexTreeNodeMemory*	pcNode;
	int						iIndex;
};


struct SIndexTreeMemoryIterator
{
	char	pvKey[MAX_KEY_SIZE];
	int		iKeyLength;
	int		iIndex;
};


class CIndexTreeMemory : public CIndexTree
{
friend class CIndexTreeWriter;
friend class CIndexTreeMemoryIterator;
protected:
	CIndexTreeNodeMemory*	mpcRoot;
	int						miSize;
	CDataFree*				mpcDataFree;
	//CCountingAllocator		mcMalloc;

public:
	void					Init(void);
	void					Init(EIndexKeyReverse eKeyReverse);
	void					Init(EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int	iMaxKeySize);
	void					Init(CIndexTreeConfig* pcConfig);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize);
	void*					Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	uint16					GetDataSize(void* pvKey, int iKeySize);

	void					FindAll(CArrayVoidPtr* papvElements);
	int						GetKey(void* pvData, char* pvDestKey, int iDestKeySize);

	int						NumElements(void);
	void					SetDataFreeCallback(CDataFree* pcDataFree);

	BOOL					StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);
	BOOL					Iterate(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);
	BOOL					StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* puiDataSize);
	BOOL					UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* puiDataSize);
	BOOL					StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize);
	BOOL					UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize);

	size_t					ByteSize(void);

	BOOL					Write(CFileWriter* pcFileWriter);
	BOOL					Read(CFileReader* pcFileReader);

	BOOL					WriteConfig(CFileWriter* pcFileWriter);  //Should be protected
	BOOL					ReadConfig(CFileReader* pcFileReader);

	BOOL					ValidateIndexTree(void);
	CIndexTreeNodeMemory*	GetRoot(void);

	size_t					GetUserMemorySize(void);
	void					Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					Dump(void);

protected:
	CIndexTreeNodeMemory*	GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);
	int						GetNodeKeySize(CIndexTreeNode* pcNode);
	int						GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize);
	uint16					GetNodeDataSize(CIndexTreeNode* pcNode);
	uint16					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize);

	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent, unsigned char uiIndexInParent);
	BOOL					Remove(CIndexTreeNodeMemory* pcCurrent);

	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, unsigned char uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForRemove(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize);

	CIndexTreeNodeMemory*	GetOrAllocateKey(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	SetNodeData(CIndexTreeNodeMemory* pcCurrent, void* pvData, uint16 uiDataSize);

	void					RemapNodePointers(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, unsigned char uiIndexInParent);
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
	BOOL					ValidateNodeTree(void);
	BOOL					RecurseValidateNodeTree(CIndexTreeRecursor* pcCursor);

	int						NumAllocatedNodes(void);

	BOOL					StepNext(SIndexTreeMemoryUnsafeIterator* psIterator);

	BOOL					ValidateSize(void);
	int						RecurseSize(void);

	void					PrintChildren(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeMemory* pcCurrent, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize);
	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags, BOOL bShowSize);
	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed);
	CIndexTreeNodeMemory*	DebugNode(CChars* pszDest, CIndexTreeNodeMemory* pcParent, int uiIndexInParent, BOOL bShowFlags, BOOL bShowSize);
};


#endif // !__INDEX_TREE_MEMORY_H__

