#include "BaseLib/SystemAllocator.h"
#include "BaseLib/NullAllocator.h"
#include "BaseLib/FreeListAllocator.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestAllocatorSystem(void)
{
	CSystemAllocator	cAlloc;
	void*				pv;
	void*				pv2;

	pv = cAlloc.Malloc(100);
	AssertNotNull(pv);

	pv2 = cAlloc.Realloc(pv, 10000);
	AssertNotNull(pv2);

	cAlloc.Free(pv2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestAllocatorNull(void)
{
	CNullAllocator	cAlloc;
	void*			pv;

	pv = cAlloc.Malloc(100);
	AssertNull(pv);
	pv = cAlloc.Malloc(100);
	AssertNull(pv);
	pv = cAlloc.Malloc(100);
	AssertNull(pv);

	pv = cAlloc.Realloc((void*)0x73984, 10000);
	AssertNull(pv);
	pv = cAlloc.Realloc(NULL, 10000);
	AssertNull(pv);
	pv = cAlloc.Realloc((void*)(size)0xffffffff, 10000);
	AssertNull(pv);

	cAlloc.Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestAllocatorFreeList(void)
{
	CFreeList*			pcBlock;
	CFreeListAllocator	cAlloc;
	void*				pv1;
	void*				pv2;
	void*				pv3;
	void*				pv4;
	void*				pv;

	cAlloc.Init(256);
	pcBlock = cAlloc.GetFreeList();
	AssertSize(0, pcBlock->NumElements());

	pv1 = cAlloc.Malloc(256);
	AssertNotNull(pv1);
	AssertSize(1, pcBlock->NumElements());
	AssertPointer(pcBlock->Get(0), pv1);

	pv2 = cAlloc.Malloc(255);
	AssertNotNull(pv2);
	AssertSize(2, pcBlock->NumElements());
	AssertPointer(pcBlock->Get(1), pv2);

	pv3 = cAlloc.Malloc(1);
	AssertNotNull(pv3);
	AssertSize(3, pcBlock->NumElements());
	AssertPointer(pcBlock->Get(2), pv3);

	pv4 = cAlloc.Malloc(256);
	AssertNotNull(pv4);
	AssertSize(4, pcBlock->NumElements());
	AssertPointer(pcBlock->Get(3), pv4);

	pv = cAlloc.Realloc(pv1, 255);
	AssertPointer(pv1, pv);
	AssertSize(4, pcBlock->NumElements());

	pv = cAlloc.Malloc(257);
	AssertNull(pv);
	AssertSize(4, pcBlock->NumElements());

	cAlloc.Free(pv2);
	AssertSize(3, pcBlock->NumElements());
	cAlloc.Free(pv1);
	AssertSize(2, pcBlock->NumElements());
	cAlloc.Free(pv4);
	AssertSize(1, pcBlock->NumElements());
	cAlloc.Free(pv3);
	AssertSize(0, pcBlock->NumElements());

	cAlloc.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestAllocator(void)
{
	BeginTests();

	TestAllocatorSystem();
	TestAllocatorNull();
	TestAllocatorFreeList();

	TestStatistics();
}

