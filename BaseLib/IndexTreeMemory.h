#ifndef __INDEX_TREE_MEMORY_H__
#define __INDEX_TREE_MEMORY_H__
#include "PrimitiveTypes.h"
#include "DataFree.h"
#include "DataIO.h"
#include "IndexTreeNodeMemory.h"
#include "IndexTreeRecursor.h"
#include "IndexTreeMemoryConfig.h"
#include "IndexTree.h"


struct SIndexTreeMemoryUnsafeIterator
{
	CIndexTreeNodeMemory*	pcNode;
	size					iIndex;
};


struct SIndexTreeMemoryIterator
{
	uint8	pvKey[MAX_KEY_SIZE];
	size	iKeyLength;
	size	iIndex;
};


struct SIndexIOElement
{
	size	iKeySize;
	size	iValueSize;
	void*	pvData;

	void Init(size iKeySize, size iValueSize, void* pvData);
};


class CIndexTreeDataSize
{
public:
	virtual size AdjustDataSize(void* pvValue, size iValueSize) =0;
};


class CIndexTreeMemory : public CIndexTree, public CDataIO, public CIndexTreeDataSize
{
friend class CIndexTreeWriter;
friend class CIndexTreeMemoryIterator;
protected:
	CIndexTreeNodeMemory*	mpcRoot;
	size					miSize;
	CDataFree*				mpcDataFree;
	CDataIO*				mpcDataIO;
	CIndexTreeDataSize*		mpcDataSize;

public:
	void					Init(void);
	void					Init(EIndexKeyReverse eKeyReverse);
	void					Init(EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize);
	void					Init(CLifeInit<CMallocator> cMalloc, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);
	void					Init(CIndexTreeConfig* pcConfig, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	void					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size iMaxDataSize, size iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);
	bool					Kill(void);

	bool					Get(uint8* pvKey, size iKeySize, void* pvDestData, size* puiDataSize, size uiMaxDataSize);
	bool					GetLongestPartial(uint8* pvKey, size iKeySize, void* pvDestData, size* puiDataSize, size uiMaxDataSize);
	void*					Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool					Remove(uint8* pvKey, size iKeySize);
	bool					HasKey(uint8* pvKey, size iKeySize);
	size					GetDataSize(uint8* pvKey, size iKeySize);

	void					FindAll(CArrayVoidPtr* papvElements);
	size					GetKey(void* pvData, uint8* pvDestKey, size iDestKeySize);

	size					NumElements(void);
	void					SetDataFreeCallback(CDataFree* pcDataFree);
	void					SetDataIOCallback(CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);

	bool					StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* puiDataSize, size uiMaxDataSize);
	bool					Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* puiDataSize, size uiMaxDataSize);
	bool					StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* puiDataSize);
	bool					Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* puiDataSize);
	bool					StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize);
	bool					Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size* puiDataSize);

	size					ByteSize(void);

	bool					Write(CFileWriter* pcFileWriter);
	bool					Read(CFileReader* pcFileReader);
	bool					Read(CFileReader* pcFileReader, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);

	bool					WriteConfig(CFileWriter* pcFileWriter);
	bool					ReadConfig(CFileReader* pcFileReader, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize);
	bool					WriteElement(CFileWriter* pcFileWriter, void* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool					ReadElement(CFileReader* pcFileReader);

	bool					WriteData(CFileWriter* pcFileWriter, void* pvData) override;
	bool					ReadData(CFileReader* pcFileReader, void* pvData) override;

	bool					ValidateIndexTree(void);
	CIndexTreeNodeMemory*	GetRoot(void);

	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	CIndexTreeNodeMemory*	GetNode(uint8* pvKey, size iKeySize);
	CIndexTreeNodeMemory*	GetNodeFromLongestPartialKey(uint8* pvKey, size iKeySize);
	CIndexTreeNodeMemory*	GetNodeForData(void* pvData);
	size					GetNodeKeySize(CIndexTreeNode* pcNode);
	size					GetNodeKey(CIndexTreeNode* pcNode, uint8* pvDestKey, size iDestKeySize);
	size					GetNodeDataSize(CIndexTreeNode* pcNode);
	size					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, size iDestDataSize);
	bool					GetNodeData(CIndexTreeNodeMemory* pcNode, size& uiDataSize, size* puiDataSize, void* pvDestData, size uiMaxDataSize);

	void					RecurseKill(CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	AllocateRoot(void);
	CIndexTreeNodeMemory*	AllocateNode(CIndexTreeNodeMemory* pcParent, size uiIndexInParent);
	bool					Remove(CIndexTreeNodeMemory* pcCurrent);

	CIndexTreeNodeMemory*	ReallocateNodeForIndex(CIndexTreeNodeMemory* pcNode, size uiIndex);
	CIndexTreeNodeMemory*	ReallocateNodeForLargerData(CIndexTreeNodeMemory* pcNode, void* pvData, size uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForSmallerData(CIndexTreeNodeMemory* pcNode, void* pvData, size uiDataSize);
	CIndexTreeNodeMemory*	ReallocateNodeForRemove(CIndexTreeNodeMemory* pcNode, size tNewNodeSize);

	CIndexTreeNodeMemory*	GetOrAllocateKey(uint8* pvKey, size iKeySize);
	CIndexTreeNodeMemory*	SetNodeData(CIndexTreeNodeMemory* pcCurrent, void* pvData, size uiDataSize);

	void					RemapNodePointers(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNode);

	CIndexTreeNodeMemory*	SetOldWithCurrent(CIndexTreeNodeMemory* pcParent, size uiIndexInParent);
	void					RecurseFindAll(CIndexTreeNodeMemory* pcNode, CArrayVoidPtr* papvElements);
	size					RecurseSize(CIndexTreeNodeMemory* pcNode);
	size					RecurseNumNodes(CIndexTreeNodeMemory* pcNode);
	size					CountListSize(void);
	size					RecurseCountListSize(CIndexTreeNodeMemory* pcNode);
	size					RecurseByteSize(CIndexTreeNodeMemory* pcNode);
	bool					ValidateLimits(void);
	bool					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);
	bool					ValidateParentIndex(void);
	bool					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor);
	bool					ValidateNodeTree(void);
	bool					RecurseValidateNodeTree(CIndexTreeRecursor* pcCursor);

	size					NumAllocatedNodes(void);

	bool					StepNext(SIndexTreeMemoryUnsafeIterator* psIterator);

	bool					ValidateSize(void);
	size					RecurseSize(void);

	void					PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeMemory* pcCurrent, size uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					RecursePrintNodes(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize);
	void					DebugKey(CChars* pszDest, uint8* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);
	CIndexTreeNodeMemory*	DebugNode(CChars* pszDest, CIndexTreeNodeMemory* pcParent, size uiIndexInParent, bool bShowFlags, bool bShowSize);

	size					AdjustDataSize(void* pvValue, size iValueSize);
};


#endif // __INDEX_TREE_MEMORY_H__

