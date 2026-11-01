#include "BaseLib/ArrayInt64.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayLongRemoveDuplicates(void)
{
	CArrayInt64	cArray;
	bool		bRemoved;

	cArray.Init();
	cArray.AddList(0LL, 1LL, 2LL, 3LL, 4LL, 5LL, 6LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(6, cArray.NumElements());
	AssertSize(8, cArray.AllocatedElements());
	AssertSize(4, cArray.ChunkSize());
	AssertSize(sizeof(int64), cArray.ElementSize());
	AssertFalse(bRemoved);
	cArray.Kill();

	cArray.Init();
	cArray.AddList(0LL, 1LL, 2LL, 3LL, 4LL, 5LL, 5LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(5, cArray.NumElements());
	AssertSize(8, cArray.AllocatedElements());
	AssertSize(4, cArray.ChunkSize());
	AssertTrue(bRemoved);
	cArray.Kill();

	cArray.Init();
	cArray.AddList(0LL, 2LL, 2LL, 3LL, 4LL, 5LL, 6LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(5, cArray.NumElements());
	AssertSize(8, cArray.AllocatedElements());
	AssertSize(4, cArray.ChunkSize());
	AssertTrue(bRemoved);
	cArray.Kill();

	cArray.Init();
	cArray.AddList(0LL, 2LL, 2LL, 3LL, 4LL, 5LL, 5LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(4, cArray.NumElements());
	AssertSize(4, cArray.AllocatedElements());
	AssertTrue(bRemoved);
	cArray.Kill();

	cArray.Init();
	cArray.AddList(0LL, 1LL, 1LL, 3LL, 3LL, 3LL, 3LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(2, cArray.NumElements());
	AssertSize(4, cArray.AllocatedElements());
	AssertSize(4, cArray.ChunkSize());
	AssertTrue(bRemoved);
	cArray.Kill();

	cArray.Init();
	cArray.AddList(0LL, 1LL, 1LL, 1LL, 1LL, 1LL, 1LL, 0LL);
	bRemoved = cArray.RemoveDuplicatesFromSorted();
	AssertSize(1, cArray.NumElements());
	AssertSize(4, cArray.AllocatedElements());
	AssertSize(4, cArray.ChunkSize());
	AssertTrue(bRemoved);
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayLong(void)
{
	BeginTests();

	TestArrayLongRemoveDuplicates();

	TestStatistics();

}

