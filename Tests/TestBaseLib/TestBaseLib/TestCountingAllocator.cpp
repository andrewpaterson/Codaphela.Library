#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/CountingAllocator.h"
#include "BaseLib/ScratchPadAllocator.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TrackingAllocator.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeSystemAllocator(void)
{
	CCountingAllocator	cCounting;
	void*				pv1;
	void*				pv2;
	void*				pv3;
	void*				pv4;
	size				iHeaderSize;

	cCounting.Init(&gcSystemAllocator);
	iHeaderSize = sizeof(SCountingMemoryAllocation);
	AssertSize(iHeaderSize, cCounting.SizeOffset());

	AssertSize(0, cCounting.AllocatedUserSize());
	AssertSize(0, cCounting.AllocatedSystemSize());

	pv1 = cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertSize(100 + iHeaderSize, cCounting.AllocatedSystemSize());

	pv2 = cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertSize(300 + iHeaderSize * 2, cCounting.AllocatedSystemSize());

	pv3 = cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertSize(350 + iHeaderSize * 3, cCounting.AllocatedSystemSize());

	pv4 = cCounting.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertSize(1300 + iHeaderSize * 3, cCounting.AllocatedSystemSize());

	cCounting.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertSize(1200 + iHeaderSize * 2, cCounting.AllocatedSystemSize());

	cCounting.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertSize(1000 + iHeaderSize, cCounting.AllocatedSystemSize());

	cCounting.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertSize(0, cCounting.AllocatedSystemSize());

	cCounting.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeMemoryAllocator(void)
{
	CCountingAllocator	cCounting;
	CMemoryAllocator	cMemory;
	void*				pv1;
	void*				pv2;
	void*				pv3;
	void*				pv4;
	size				iHeaderSize;

	cMemory.Init();
	cCounting.Init(&cMemory);
	iHeaderSize = sizeof(SGeneralMemoryAllocation);
	AssertSize(iHeaderSize, cCounting.SizeOffset());

	AssertSize(0, cCounting.AllocatedUserSize());
	AssertSize(0, cCounting.AllocatedSystemSize());

	pv1 = cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertSize(100 + iHeaderSize, cCounting.AllocatedSystemSize());

	pv2 = cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertSize(300 + iHeaderSize * 2, cCounting.AllocatedSystemSize());

	pv3 = cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertSize(350 + iHeaderSize * 3, cCounting.AllocatedSystemSize());

	pv4 = cCounting.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertSize(1300 + iHeaderSize * 3, cCounting.AllocatedSystemSize());

	cCounting.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertSize(1200 + iHeaderSize * 2, cCounting.AllocatedSystemSize());

	cCounting.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertSize(1000 + iHeaderSize, cCounting.AllocatedSystemSize());

	cCounting.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertSize(0, cCounting.AllocatedSystemSize());

	cCounting.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeScratchPadAllocator(void)
{
	CCountingAllocator		cCounting;
	CScratchPadAllocator	cScratchPad;

	cScratchPad.Init();
	cCounting.Init(&cScratchPad);
	AssertSize(0, cCounting.AllocatedUserSize());

	cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());

	cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());

	cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertSize(350 + sizeof(SSPNode) * 3, cCounting.AllocatedSystemSize());
	AssertSize(350 + sizeof(SSPNode) * 3, cScratchPad.GetScratchPad()->GetUsedSize());

	cCounting.Kill();
	cScratchPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeGlobalMemoryAllocator(void)
{
	CCountingAllocator		cCounting;
	void*					pv1;
	void*					pv2;
	void*					pv3;
	void*					pv4;

	MemoryInit();

	cCounting.Init(&gcMemoryAllocator);
	AssertSize(0, cCounting.AllocatedUserSize());

	pv1 = cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());

	pv2 = cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());

	pv3 = cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());

	pv4 = cCounting.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());

	cCounting.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());

	cCounting.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());

	cCounting.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());

	cCounting.Kill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeTrackingSystemAllocator(void)
{
	CCountingAllocator	cCounting;
	CTrackingAllocator	cTracking;
	void*				pv1;
	void*				pv2;
	void*				pv3;
	void*				pv4;

	cTracking.Init(&gcSystemAllocator);
	cCounting.Init(&cTracking);
	AssertSize(sizeof(SCountingMemoryAllocation), cCounting.SizeOffset());
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	pv1 = cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	pv2 = cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	pv3 = cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	pv4 = cCounting.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	cCounting.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	cCounting.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	cCounting.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	cCounting.Kill();
	cTracking.Kill();

	cCounting.Init(&gcSystemAllocator);
	cTracking.Init(&cCounting);
	AssertSize(sizeof(SCountingMemoryAllocation), cTracking.SizeOffset());
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	pv1 = cTracking.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	pv2 = cTracking.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	pv3 = cTracking.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	pv4 = cTracking.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	cTracking.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	cTracking.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	cTracking.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	cCounting.Kill();
	cTracking.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocatorSizeTrackingGlobalMemoryAllocator(void)
{
	CCountingAllocator		cCounting;
	CTrackingAllocator		cTracking;
	void*					pv1;
	void*					pv2;
	void*					pv3;
	void*					pv4;

	MemoryInit();
	cTracking.Init(&gcMemoryAllocator);
	cCounting.Init(&cTracking);
	AssertSize(sizeof(SGeneralMemoryAllocation), cCounting.SizeOffset());
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	pv1 = cCounting.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	pv2 = cCounting.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	pv3 = cCounting.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	pv4 = cCounting.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	cCounting.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	cCounting.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	cCounting.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	cCounting.Kill();
	cTracking.Kill();
	MemoryKill();

	MemoryInit();
	cCounting.Init(&gcMemoryAllocator);
	cTracking.Init(&cCounting);
	AssertSize(sizeof(SGeneralMemoryAllocation), cCounting.SizeOffset());
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	pv1 = cTracking.Malloc(100);
	AssertSize(100, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	pv2 = cTracking.Malloc(200);
	AssertSize(300, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	pv3 = cTracking.Malloc(50);
	AssertSize(350, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	pv4 = cTracking.Realloc(pv3, 1000);
	AssertSize(1300, cCounting.AllocatedUserSize());
	AssertInt(3, cTracking.AllocatedCount());

	cTracking.Free(pv1);
	AssertSize(1200, cCounting.AllocatedUserSize());
	AssertInt(2, cTracking.AllocatedCount());

	cTracking.Free(pv2);
	AssertSize(1000, cCounting.AllocatedUserSize());
	AssertInt(1, cTracking.AllocatedCount());

	cTracking.Free(pv4);
	AssertSize(0, cCounting.AllocatedUserSize());
	AssertInt(0, cTracking.AllocatedCount());

	cCounting.Kill();
	cTracking.Kill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCountingAllocator(void)
{
	BeginTests();

	TestCountingAllocatorSizeSystemAllocator();
	TestCountingAllocatorSizeMemoryAllocator();
	TestCountingAllocatorSizeScratchPadAllocator();
	TestCountingAllocatorSizeGlobalMemoryAllocator();
	TestCountingAllocatorSizeTrackingSystemAllocator();
	TestCountingAllocatorSizeTrackingGlobalMemoryAllocator();

	TestStatistics();
}

