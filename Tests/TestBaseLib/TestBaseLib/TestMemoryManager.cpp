#include "BaseLib/MemoryManager.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/StdRandom.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerContiguousAllocations(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	uint32			uiMemorySize;

	uiMemorySize = (uint32)(10 + 20 + 30 + 3 * MM_NODE_HEADER_SIZE);
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(10);
	pvMem2 = cMemory.Allocate(20);
	pvMem3 = cMemory.Allocate(30);
	pvMem4 = cMemory.Allocate(1);
	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNull(pvMem4);
	AssertSize(10 + MM_NODE_HEADER_SIZE, (size)((ptr)pvMem2 - (ptr)pvMem1));
	AssertSize(20 + MM_NODE_HEADER_SIZE, (size)((ptr)pvMem3 - (ptr)pvMem2));

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMMNode* AssertNode(SMMNode* psNode, uint32 uiExpectedSize, bool bUsed)
{
	size uiActualSize;

	uiActualSize = GetMMNodeDataSize(psNode) - MM_NODE_HEADER_SIZE;
	AssertSize(uiExpectedSize, uiActualSize);
	AssertBool(bUsed, FixBool(psNode->uiFlags & MM_NODE_USED));
	return (SMMNode*)psNode->psNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMMNode* AssertNode(SMMNode* psNode, uint32 uiExpectedUserSize, uint32 uiExpectedNodeSize, bool bUsed)
{
	size uiActualUserSize;
	size uiActualNodeSize;

	uiActualUserSize = GetMMNodeDataSize(psNode) - MM_NODE_HEADER_SIZE;
	uiActualNodeSize = (size)((ptr)(psNode->psNext) - (ptr)psNode) - MM_NODE_HEADER_SIZE;
	AssertSize(uiExpectedUserSize, uiActualUserSize);
	AssertSize(uiExpectedNodeSize, uiActualNodeSize);
	AssertBool(bUsed, FixBool(psNode->uiFlags & MM_NODE_USED));
	return (SMMNode*)psNode->psNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerDeallocate1(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	uint32			uiMemorySize;
	SMMNode*		psNode;

	uiMemorySize = (uint32)(4 * (10 + MM_NODE_HEADER_SIZE));
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(10);
	pvMem2 = cMemory.Allocate(10);
	pvMem3 = cMemory.Allocate(10);
	pvMem4 = cMemory.Allocate(10);

	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNotNull(pvMem4);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 10, true);
	psNode = AssertNode(psNode, 10, false);
	psNode = AssertNode(psNode, 10, true);
	psNode = AssertNode(psNode, 10, true);
	AssertNull(psNode);
	
	cMemory.Deallocate(pvMem1);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(10 + 10 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 10, true);
	psNode = AssertNode(psNode, 10, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem3);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(10 + 10 + 10 + 2 * MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 10, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem4);
	psNode = cMemory.StartIteration();
	AssertNull(psNode);

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerDeallocate2(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	void*			pvMem5;
	uint32			uiMemorySize;
	SMMNode*		psNode;

	uiMemorySize = (uint32)(5 * (8 + MM_NODE_HEADER_SIZE));
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(8);
	pvMem2 = cMemory.Allocate(8);
	pvMem3 = cMemory.Allocate(8);
	pvMem4 = cMemory.Allocate(8);
	pvMem5 = cMemory.Allocate(8);

	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNotNull(pvMem4);
	AssertNotNull(pvMem5);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);
	
	cMemory.Deallocate(pvMem4);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem3);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, (uint32)(8 + 8 + 8 + 2 * MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem1);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(8 + 8 + 8 + 8 + 3 * MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem5);
	psNode = cMemory.StartIteration();
	AssertNull(psNode);

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerDeallocateAllocate1(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	uint32			uiMemorySize;
	SMMNode*		psNode;

	uiMemorySize = (uint32)(4 * (8 + MM_NODE_HEADER_SIZE) + 20);
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(8);
	pvMem2 = cMemory.Allocate(8);
	pvMem3 = cMemory.Allocate(8);
	pvMem4 = cMemory.Allocate(8);

	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNotNull(pvMem4);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem2 = cMemory.Allocate(8);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem3);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, (uint32)(8 + 8 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem2 = cMemory.Allocate(16);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 16, (uint32)(8 + 8 + MM_NODE_HEADER_SIZE), true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, (uint32)(8 + 8 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem2 = cMemory.Allocate(6);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 6, 6, true);
	psNode = AssertNode(psNode, 10, false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerDeallocateAllocate2(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	uint32			uiMemorySize;
	SMMNode*		psNode;

	uiMemorySize = (uint32)(4 * (8 + MM_NODE_HEADER_SIZE) + 20);
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(8);
	pvMem2 = cMemory.Allocate(8);
	pvMem3 = cMemory.Allocate(8);
	pvMem4 = cMemory.Allocate(8);

	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNotNull(pvMem4);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem3);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, (uint32)(8 + 8 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem2 = cMemory.Allocate(16);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 16, (uint32)(8 + 8 + MM_NODE_HEADER_SIZE), true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem4);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 16, true);
	AssertNull(psNode);

	pvMem2 = cMemory.Allocate(6);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 16, 16, true);
	psNode = AssertNode(psNode, 6, true);
	AssertNull(psNode);

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerDeallocateAllocate3(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pvMem1;
	void*			pvMem2;
	void*			pvMem3;
	void*			pvMem4;
	uint32			uiMemorySize;
	SMMNode*		psNode;

	uiMemorySize = (uint32)(12 + 8 + 12 + 8 + 4 * MM_NODE_HEADER_SIZE);
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);
	cMemory.DisableEmptyCache();

	pvMem1 = cMemory.Allocate(12);
	pvMem2 = cMemory.Allocate(8);
	pvMem3 = cMemory.Allocate(12);
	pvMem4 = cMemory.Allocate(8);

	AssertNotNull(pvMem1);
	AssertNotNull(pvMem2);
	AssertNotNull(pvMem3);
	AssertNotNull(pvMem4);

	cMemory.Deallocate(pvMem1);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 12, false);
	psNode = AssertNode(psNode, 8, true);
	psNode = AssertNode(psNode, 12, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem2);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(12 + 8 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 12, 12, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem1 = cMemory.Allocate(16);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 16, (uint32)(12 + 8 + MM_NODE_HEADER_SIZE), true);
	psNode = AssertNode(psNode, 12, 12, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem1);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(12 + 8 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 12, 12, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	pvMem1 = cMemory.Allocate(15);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 15, 15, true);
	psNode = AssertNode(psNode, 5, false);
	psNode = AssertNode(psNode, 12, 12, true);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem3);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, 15, 15, true);
	psNode = AssertNode(psNode, (uint32)(17 + MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem1);
	psNode = cMemory.StartIteration();
	psNode = AssertNode(psNode, (uint32)(32 + 2 * MM_NODE_HEADER_SIZE), false);
	psNode = AssertNode(psNode, 8, true);
	AssertNull(psNode);

	cMemory.Deallocate(pvMem4);
	psNode = cMemory.StartIteration();
	AssertNull(psNode);

	cMemory.Kill();

	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManagerEmptyCache(void)
{
	CMemoryManager	cMemory;
	void*			pvStartMemory;
	void*			pvEndMemory;
	void*			pv;
	uint32			uiMemorySize;
	uint32			uiCount;
	uint8			uiValue;
	uint32			uiAllocations;
	uint32			uiRemainingSize;
	uint32			uiUsedSize;
	uint32			uiUnusedSize;
	uint32			uiTotalSize;
	int32			i;
	int32			iIndex;
	int32			iToRemove;
	int32			j;
	CArrayVoidPtr	apvAllocations;
	CRandom			cRandom;
	void**			ppv;
	uint32			uiRemovals;
	uint32			uiExpectedAllocations;
	uint32			uiExpectedTail;
	uint32			uiExpectedAllocationsAfterRemoval;
	uint32			uiExpectedTailAfterRemoval;
	uint32			uiExpectedUsedAfterRemoval;

	//The node header holds two pointers so fewer allocations fit in a 64 bit build.
#ifdef BUILD_64
	uiExpectedAllocations = 526;
	uiExpectedTail = 102;
	uiExpectedAllocationsAfterRemoval = 328;
	uiExpectedTailAfterRemoval = 246;
	uiExpectedUsedAfterRemoval = 40906;
#else
	uiExpectedAllocations = 582;
	uiExpectedTail = 52;
	uiExpectedAllocationsAfterRemoval = 382;
	uiExpectedTailAfterRemoval = 52;
	uiExpectedUsedAfterRemoval = 42753;
#endif // BUILD_64

	uiMemorySize = 64 KB;
	pvStartMemory = calloc(1, uiMemorySize);
	pvEndMemory = RemapSinglePointer(pvStartMemory, (uiMemorySize - 1));
	cMemory.Init(pvStartMemory, pvEndMemory);

	apvAllocations.Init();

	uiCount = 100;
	uiValue = 0;
	for (;;)
	{
		pv = cMemory.Allocate(uiCount);
		if (pv == NULL)
		{
			break;
		}

		apvAllocations.Add(pv);

		memset(pv, uiValue, uiCount);
		uiValue++;
		uiCount++;
		if (uiCount > 150)
		{
			uiCount = 50;
		}
	}

	uiTotalSize = cMemory.GetTotalSize();
	AssertInt((uint32)(64 KB), uiTotalSize);
	uiAllocations = cMemory.GetNumAllocations(false);
	AssertInt(uiExpectedAllocations, uiAllocations);
	AssertSize(uiExpectedAllocations, apvAllocations.NumElements());
	uiRemainingSize = cMemory.GetRemaingTailSize();
	AssertInt(uiExpectedTail, uiRemainingSize);
	uiUsedSize = cMemory.GetUsedAllocationSize();
	AssertInt((uint32)(64 KB) - uiExpectedTail, uiUsedSize);
	uiUnusedSize = cMemory.GetUnusedAllocationSize();
	AssertInt((uint32)0, uiUnusedSize);
	AssertInt((uint32)(64 KB), uiRemainingSize + uiUsedSize + uiUnusedSize);

	uiRemovals = 0;
	cRandom.Init(3456897);
	for (i = 0; i < 100; i++)
	{
		iIndex = cRandom.Next(0, (int32)(apvAllocations.NumElements() - 1));
		iToRemove = cRandom.Next(0, 2);
		if (iIndex + iToRemove >= (int32)apvAllocations.NumElements())
		{
			iToRemove = (int32)apvAllocations.NumElements() - 1 - iIndex;
		}
		for (j = iToRemove; j >= 0; j--)
		{
			ppv = apvAllocations.Get(iIndex + j);
			pv = *ppv;
			cMemory.Deallocate(pv);
			apvAllocations.RemoveAt(iIndex + j, true);
			uiRemovals++;
		}
		uiAllocations = cMemory.GetNumAllocations(false);
		AssertInt(uiExpectedAllocations, uiAllocations + uiRemovals);
	}
	
	uiTotalSize = cMemory.GetTotalSize();
	AssertInt((uint32)(64 KB), uiTotalSize);
	uiAllocations = cMemory.GetNumAllocations(false);
	AssertInt(uiExpectedAllocationsAfterRemoval, uiAllocations);
	uiRemainingSize = cMemory.GetRemaingTailSize();
	AssertInt(uiExpectedTailAfterRemoval, uiRemainingSize);
	uiUsedSize = cMemory.GetUsedAllocationSize();
	AssertInt(uiExpectedUsedAfterRemoval, uiUsedSize);
	uiUnusedSize = cMemory.GetUnusedAllocationSize();
	AssertInt((uint32)(64 KB) - uiExpectedTailAfterRemoval - uiExpectedUsedAfterRemoval, uiUnusedSize);
	AssertInt((uint32)(64 KB), uiRemainingSize + uiUsedSize + uiUnusedSize);

	apvAllocations.Kill();

	cRandom.Kill();
	cMemory.Kill();
	free(pvStartMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryManager(void)
{
	BeginTests();

	TestMemoryManagerContiguousAllocations();
	TestMemoryManagerDeallocate1();
	TestMemoryManagerDeallocate2();
	TestMemoryManagerDeallocateAllocate1();
	TestMemoryManagerDeallocateAllocate2();
	TestMemoryManagerDeallocateAllocate3();
	TestMemoryManagerEmptyCache();

	TestStatistics();
}

