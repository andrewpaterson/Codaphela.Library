#include "BaseLib/SystemAllocator.h"
#include "BaseLib/FreeListAllocator.h"
#include "BaseLib/MemoryBlock.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryBlockAllocate(void)
{
	CMemoryBlock	cBlock;
	void*			pvMem;

	cBlock.Init();
	pvMem = cBlock.Allocate(10);
	AssertNotNull(pvMem);
	AssertPointer(pvMem, cBlock.GetMemory());
	AssertInt(10, (int)cBlock.GetSize());
	pvMem = cBlock.Allocate(20);
	AssertNotNull(pvMem);
	AssertPointer(pvMem, cBlock.GetMemory());
	AssertInt(20, (int)cBlock.GetSize());
	cBlock.Kill();
	AssertNull(cBlock.GetMemory());
	AssertInt(0, (int)cBlock.GetSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryBlockGlobalMemory(void)
{
	MemoryInit();

	CMemoryBlock	cBlock;
	void*			pvMem;

	AssertSize(0, gcMemory.NumElements());
	AssertTrue(gbMemory);

	cBlock.Init(&gcMemoryAllocator);
	pvMem = cBlock.Allocate(10);
	AssertNotNull(pvMem);
	AssertPointer(pvMem, cBlock.GetMemory());
	AssertInt(10, (int)cBlock.GetSize());
	AssertSize(1, gcMemory.NumElements());

	pvMem = cBlock.Allocate(20);
	AssertNotNull(pvMem);
	AssertPointer(pvMem, cBlock.GetMemory());
	AssertInt(20, (int)cBlock.GetSize());
	AssertSize(1, gcMemory.NumElements());

	cBlock.Kill();
	AssertNull(cBlock.GetMemory());
	AssertInt(0, (int)cBlock.GetSize());
	AssertSize(0, gcMemory.NumElements());

	MemoryKill();
	AssertFalse(gbMemory);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryBlock(void)
{
	BeginTests();

	TestMemoryBlockAllocate();
	TestMemoryBlockGlobalMemory();

	TestStatistics();
}

