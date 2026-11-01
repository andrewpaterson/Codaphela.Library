#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/TrackingAllocator.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrackingAllocatorCount(void)
{
	CTrackingAllocator	cTracking;
	CMemoryAllocator	cMemory;
	void*				pv1;
	void*				pv2;
	void*				pv3;
	void*				pv4;

	cMemory.Init();
	cTracking.Init(&cMemory);
	AssertInt(0, cTracking.AllocatedCount());

	pv1 = cTracking.Malloc(100);
	AssertInt(1, cTracking.AllocatedCount());

	pv2 = cTracking.Malloc(200);
	AssertInt(2, cTracking.AllocatedCount());

	pv3 = cTracking.Malloc(50);
	AssertInt(3, cTracking.AllocatedCount());

	pv4 = cTracking.Realloc(pv3, 1000);
	AssertInt(3, cTracking.AllocatedCount());

	cTracking.Free(pv1);
	AssertInt(2, cTracking.AllocatedCount());

	cTracking.Free(pv2);
	AssertInt(1, cTracking.AllocatedCount());

	cTracking.Free(pv4);
	AssertInt(0, cTracking.AllocatedCount());

	cTracking.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrackingAllocator(void)
{
	BeginTests();

	TestTrackingAllocatorCount();

	TestStatistics();
}

