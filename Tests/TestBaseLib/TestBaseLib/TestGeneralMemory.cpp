#include "BaseLib/GeneralMemory.h"
#include "TestLib/Assert.h"


#define MEM_TEST_INT_1 0x3f47a921
#define MEM_TEST_INT_2 0x64d80c23


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemorySizeof(void)
{
#ifdef BUILD_64
	AssertSize(320, sizeof(CGeneralMemory));
#elif defined(SIZE_16)
	AssertSize(120, sizeof(CGeneralMemory));
#else
	AssertSize(160, sizeof(CGeneralMemory));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryAdd(void)
{
	CGeneralMemory				cMemory;
	void*						pv1;
	void*						pv2;
	void*						pv4;
	void*						pv5;
	void*						pv8;
	void*						pv9;
	void*						pv;
	SGeneralMemoryAllocation*	psAlloc;
	CFreeList*					pcList;
	CFreeList*					pcListSame;

	cMemory.Init();

	pv1 = cMemory.Add(1);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv1);
	AssertSize(1, psAlloc->uiSize);
	AssertSize(1, cMemory.GetSize(pv1));
	pcList = cMemory.GetFreeList(1, 4, 0);
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	pcListSame = cMemory.GetFreeList(8, 4, 0);
	AssertPointer(pcList, pcListSame);
#ifdef BUILD_64
	AssertLSize(65640, cMemory.ByteSize());
#elif defined(SIZE_16)
	AssertLSize(65588, cMemory.ByteSize());
#else
	AssertLSize(65592, cMemory.ByteSize());
#endif // BUILD_64

	pv2 = cMemory.Grow(pv1, 2);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv2);
	AssertSize(2, psAlloc->uiSize);
	AssertSize(2, cMemory.GetSize(pv2));
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	AssertPointer(pv1, pv2);
#ifdef BUILD_64
	AssertLSize(65640, cMemory.ByteSize());
#elif defined(SIZE_16)
	AssertLSize(65588, cMemory.ByteSize());
#else
	AssertLSize(65592, cMemory.ByteSize());
#endif // BUILD_64

	pv4 = cMemory.Grow(pv2, 4);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv4);
	AssertSize(4, psAlloc->uiSize);
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	AssertPointer(pv1, pv4);
	*((int32*)pv4) = MEM_TEST_INT_1;

	pv5 = cMemory.Grow(pv4, 5);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv4);
	AssertSize(5, psAlloc->uiSize);
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	AssertPointer(pv1, pv5);
	AssertInt(MEM_TEST_INT_1, *((int32*)pv5));

	pv8 = cMemory.Grow(pv5, 8);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv8);
	AssertSize(8, psAlloc->uiSize);
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	AssertPointer(pv1, pv8);
	AssertInt(MEM_TEST_INT_1, *((int32*)pv8));
	((int32*)pv8)[1] = MEM_TEST_INT_2;

	pv9 = cMemory.Grow(pv8, 9);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv9);
	AssertSize(9, psAlloc->uiSize);
	pcList = cMemory.GetFreeList(9, 4, 0);	AssertNotNull(pcList);
	AssertPointer(pcList, CFreeList::GetSlabNode(psAlloc)->pcList);
	AssertFalse(pv1 == pv9);
	AssertInt(MEM_TEST_INT_1, *((int32*)pv9));
	AssertInt(MEM_TEST_INT_2, ((int32*)pv9)[1]);
	pcList = cMemory.GetFreeList(8, 4, 0);
	AssertNotNull(pcList);
	AssertSize(0, pcList->NumElements());
	cMemory.Compact();
	pcList = cMemory.GetFreeList(8, 4, 0);
	AssertNull(pcList);
#ifdef BUILD_64
	AssertLSize(65640, cMemory.ByteSize());
#elif defined(SIZE_16)
	AssertLSize(65588, cMemory.ByteSize());
#else
	AssertLSize(65592, cMemory.ByteSize());
#endif // BUILD_64

	cMemory.Kill();

	cMemory.Init();

	pv4 = cMemory.Add(4);
	*((int32*)pv4) = MEM_TEST_INT_1;
	pv8 = cMemory.Grow(pv4, 8);
	((int32*)pv8)[1] = MEM_TEST_INT_2;

	pv = cMemory.Add(4);
	*((int32*)pv) = MEM_TEST_INT_1;
	AssertInt(MEM_TEST_INT_2, ((int32*)pv8)[1]);

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryLargeAdd(void)
{
	CGeneralMemory	cMemory;
	CFreeList*		pcList;

	cMemory.Init();
	pcList = cMemory.TestGetFreeListsHead();
	AssertNull(pcList);

	cMemory.Add(MEMORY_FREE_LIST_SIZE_LIMIT - sizeof(SGeneralMemoryAllocation));
	pcList = cMemory.TestGetFreeListsHead();
	AssertNotNull(pcList);
#ifdef BUILD_64
	AssertLSize(65640, cMemory.ByteSize());
#elif defined(SIZE_16)
	AssertLSize(65588, cMemory.ByteSize());
#else
	AssertLSize(65592, cMemory.ByteSize());
#endif // BUILD_64
	cMemory.Kill();

	cMemory.Init();

	cMemory.Add(MEMORY_FREE_LIST_SIZE_LIMIT - sizeof(SGeneralMemoryAllocation) + 1);
	pcList = cMemory.TestGetFreeListsHead();
	AssertNull(pcList);
#ifdef BUILD_64
	AssertSize(2124, cMemory.ByteSize());
#else
	AssertSize(2088, cMemory.ByteSize());
#endif // BUILD_64

	cMemory.TestGetLargeListsHead();

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupTestGeneralMemoryRemove(CGeneralMemory* pcMemory)
{
	void*	pv;

	pcMemory->Init(4, false);
	pcMemory->GetFreeListParams()->AddParamBlock(8, 0, 4);
	pcMemory->GetFreeListParams()->AddParamBlock(16, 8, 4);
	pcMemory->GetFreeListParams()->AddParamBlock(24, 16, 4);

	pv = pcMemory->Add(10);
	AssertSize(1, pcMemory->NumElements());
	pcMemory->Remove(pv);
	AssertSize(0, pcMemory->NumElements());

	pcMemory->Add(10);
	pcMemory->Add(11);
	pv = pcMemory->Add(12);
	pcMemory->Add(13);

	pcMemory->Add(9);
	pcMemory->Add(14);
	pcMemory->Add(10);
	pcMemory->Add(9);

	pcMemory->Add(9);
	pcMemory->Add(14);

	pcMemory->Remove(pv);
	AssertSize(9, pcMemory->NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryRemoveAllByArray(void)
{
	CGeneralMemory		cMemory;
	CArrayVoidPtr		apv;
	SMemoryIterator		sIter;
	SMemory				sMem;

	SetupTestGeneralMemoryRemove(&cMemory);

	apv.Init();
	sMem = cMemory.StartIteration(&sIter);
	while (sMem.bValid)
	{
		apv.Add(sMem.pvMem);
		sMem = cMemory.Iterate(&sIter);
	}

	AssertSize(9, apv.NumElements());

	cMemory.RemoveMultiple(&apv);
	AssertSize(0, cMemory.NumElements());

	apv.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryRemoveHalfByArray(void)
{
	CGeneralMemory		cMemory;
	CArrayVoidPtr		apv;
	SMemoryIterator		sIter;
	bool				bAdd;
	SMemory				sMem;

	SetupTestGeneralMemoryRemove(&cMemory);

	apv.Init();
	sMem = cMemory.StartIteration(&sIter);
	bAdd = true;
	while (sMem.bValid)
	{
		if (bAdd)
		{
			apv.Add(sMem.pvMem);
		}
		bAdd = !bAdd;
		sMem = cMemory.Iterate(&sIter);
	}

	AssertSize(5, apv.NumElements());

	cMemory.RemoveMultiple(&apv);
	AssertSize(4, cMemory.NumElements());

	apv.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryRemoveNoneByArray(void)
{
	CGeneralMemory			cMemory;
	CArrayVoidPtr	apv;

	SetupTestGeneralMemoryRemove(&cMemory);

	apv.Init();
	
	cMemory.RemoveMultiple(&apv);
	AssertSize(9, cMemory.NumElements());

	apv.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryGrow(void)
{
	CGeneralMemory	cMemory;
	char			sz[27] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
	char*			sz1;
	char*			sz2;

	cMemory.Init();

	sz1 = (char*)cMemory.Add(16);
	memcpy(sz1, sz, 15);
	sz1[15] = 0;
	AssertString("ABCDEFGHIJKLMNO", sz1);

	sz2 = (char*)cMemory.Grow(sz1, 20);
	AssertFalse(sz1 == sz2);
	AssertString("ABCDEFGHIJKLMNO", sz2);

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemoryIteration(void)
{
	CGeneralMemory		cMemory;
	void*				pv8192;
	void*				pv48;
	void*				pv64;
	void*				pv9036;
	SMemoryIterator		sIterator;
	SMemory				sResult;

	cMemory.Init();
	
	sResult = cMemory.StartIteration(&sIterator);
	AssertFalse(sResult.bValid);


	pv8192 = cMemory.Add(8192);
	sResult = cMemory.StartIteration(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(8192, sResult.uiSize);
	AssertPointer(pv8192, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertFalse(sResult.bValid);


	pv48 = cMemory.Add(48);
	sResult = cMemory.StartIteration(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(48, sResult.uiSize);
	AssertPointer(pv48, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(8192, sResult.uiSize);
	AssertPointer(pv8192, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertFalse(sResult.bValid);


	pv64 = cMemory.Add(64);
	sResult = cMemory.StartIteration(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(48, sResult.uiSize);
	AssertPointer(pv48, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(64, sResult.uiSize);
	AssertPointer(pv64, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(8192, sResult.uiSize);
	AssertPointer(pv8192, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertFalse(sResult.bValid);


	cMemory.Remove(pv8192);
	sResult = cMemory.StartIteration(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(48, sResult.uiSize);
	AssertPointer(pv48, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(64, sResult.uiSize);
	AssertPointer(pv64, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertFalse(sResult.bValid);


	pv8192 = cMemory.Add(8192);
	pv9036 = cMemory.Add(9036);
	sResult = cMemory.StartIteration(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(48, sResult.uiSize);
	AssertPointer(pv48, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(64, sResult.uiSize);
	AssertPointer(pv64, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(8192, sResult.uiSize);
	AssertPointer(pv8192, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertTrue(sResult.bValid);
	AssertSize(9036, sResult.uiSize);
	AssertPointer(pv9036, sResult.pvMem);

	sResult = cMemory.Iterate(&sIterator);
	AssertFalse(sResult.bValid);

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeneralMemory(void)
{
	BeginTests();

	TestGeneralMemorySizeof();
	TestGeneralMemoryLargeAdd();
	TestGeneralMemoryAdd();
	TestGeneralMemoryGrow();
	TestGeneralMemoryRemoveAllByArray();
	TestGeneralMemoryRemoveHalfByArray();
	TestGeneralMemoryRemoveNoneByArray();
	TestGeneralMemoryIteration();

	TestStatistics();
}

