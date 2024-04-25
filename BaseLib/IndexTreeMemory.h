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
	bool					Kill(void);

	bool					Get(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize);
	bool					GetLongestPartial(void* pvKey, int iKeySize, void* pvDestData, size_t* puiDataSize, size_t uiMaxDataSize);
	void*					Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	bool					Remove(void* pvKey, int iKeySize);
	bool					HasKey(void* pvKey, int iKeySize);
	uint16					GetDataSize(void* pvKey, int iKeySize);

	void					FindAll(CArrayVoidPtr* papvElements);
	int						GetKey(void* pvData, char* pvDestKey, int iDestKeySize);

	int						NumElements(void);
	void					SetDataFreeCallback(CDataFree* pcDataFree);

	bool					StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);
	bool					Iterate(SIndexTreeMemoryIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* puiDataSize, size_t uiMaxDataSize);
	bool					StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* puiDataSize);
	bool					UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* puiDataSize);
	bool					StartUnsafeIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize);
	bool					UnsafeIterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize);

	size_t					ByteSize(void);

	bool					Write(CFileWriter* pcFileWriter);
	bool					Read(CFileReader* pcFileReader);

	bool					WriteConfig(CFileWriter* pcFileWriter);  //Should be protected
	bool					ReadConfig(CFileReader* pcFileReader);

	bool					ValidateIndexTree(void);
	CIndexTreeNodeMemory*	GetRoot(void);

	size_t					GetUserMemorySize(void);
	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	CIndexTreeNodeMemory*	GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	GetNodeFromLongestPartialKey(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);
	int						GetNodeKeySize(CIndexTreeNode* pcNode);
	int						GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize);
	uint16					GetNodeDataSize(CIndexTreeNode* pcNode);
	uint16					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize);
	bool					GetNodeData(CIndexTreeNodeMemory* pcNode, uint16& uiDataSize, size_t* puiDataSize, void* pvDestData, size_t uiMaxDataSize);

	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent, uint8 uiIndexInParent);
	bool					Remove(CIndexTreeNodeMemory* pcCurrent);

	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, uint8 uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, void* pvData, uint16 uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForRemove(CIndexTreeNodeMemory* pcNode, size_t tNewNodeSize);

	CIndexTreeNodeMemory*	GetOrAllocateKey(void* pvKey, int iKeySize);
	CIndexTreeNodeMemory*	SetNodeData(CIndexTreeNodeMemory* pcCurrent, void* pvData, uint16 uiDataSize);

	void					RemapNodePointers(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, uint8 uiIndexInParent);
	void					RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements);
	int						RecurseSize(CIndexTreeNodeMemory* pcNode);
	int						RecurseNumNodes(CIndexTreeNodeMemory* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeMemory* pcNode);
	bool					ValidateLimits(void);
	bool					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);
	bool					ValidateParentIndex(void);
	bool					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor);
	bool					ValidateNodeTree(void);
	bool					RecurseValidateNodeTree(CIndexTreeRecursor* pcCursor);

	int						NumAllocatedNodes(void);

	bool					StepNext(SIndexTreeMemoryUnsafeIterator* psIterator);

	bool					ValidateSize(void);
	int						RecurseSize(void);

	void					PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeMemory* pcCurrent, int uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize);
	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);
	CIndexTreeNodeMemory*	DebugNode(CChars* pszDest, CIndexTreeNodeMemory* pcParent, int uiIndexInParent, bool bShowFlags, bool bShowSize);
};


#endif // !__INDEX_TREE_MEMORY_H__

