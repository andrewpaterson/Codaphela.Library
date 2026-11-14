#include "BaseLib/ArrayChars.h"
#include "BaseLib/DataMemory.h"
#include "TestLib/Words.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataMemorySizeof(void)
{
#ifdef BUILD_64
	AssertSize(176, sizeof(CDataMemory));
#elif defined(SIZE_16)
	AssertSize(72, sizeof(CDataMemory));
#else
	AssertSize(88, sizeof(CDataMemory));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataMemoryAdd(void)
{
	CDataMemory		cMemory;
	size				i;
	CChars*			psz;
	void*			pv;
	size				iAllocated;
	CArrayVoidPtr	apv;

	cMemory.Init(&gcDataMemoryFreeListParams);
	AssertSize(0, cMemory.NumElements());
#ifdef BUILD_64
	AssertSize(32, cMemory.ByteSize());
#else
	AssertSize(16, cMemory.ByteSize());
#endif // BUILD_64
	AssertSize(0, cMemory.NumFreeLists());


	apv.Init();
	iAllocated = 0;
	for (i = 0; i < gaszArchaicDefinitions.NumElements(); i++)
	{
		psz = gaszArchaicDefinitions.Get(i);
		pv = cMemory.Add(psz->Length());
		strcpy((char*)pv, psz->Text());
		apv.Add(pv);

		iAllocated += psz->Length() + 1;
	}

	AssertSize(433, gaszArchaicDefinitions.NumElements());
	AssertSize(gaszArchaicDefinitions.NumElements(), cMemory.NumElements());
#ifdef BUILD_64
	AssertSize(39122, cMemory.ByteSize());
	AssertSize(9, cMemory.NumFreeLists());
#elif defined(SIZE_16)
	AssertSize(30341, cMemory.ByteSize());
	AssertSize(8, cMemory.NumFreeLists());
#else
	AssertSize(30373, cMemory.ByteSize());
	AssertSize(8, cMemory.NumFreeLists());
#endif // BUILD_64
	AssertSize(13665, iAllocated);

	for (i = 0; i < apv.NumElements(); i += 2)
	{
		pv = apv.GetPtr(i);
		cMemory.Remove(pv);
	}
	cMemory.Compact();

	AssertSize(216, cMemory.NumElements());
#ifdef BUILD_64
	AssertSize(33740, cMemory.ByteSize());
#elif defined(SIZE_16)
	AssertSize(30341, cMemory.ByteSize());
#else
	AssertSize(30373, cMemory.ByteSize());
#endif // BUILD_64
	AssertSize(8, cMemory.NumFreeLists());

	for (i = 1; i < apv.NumElements(); i += 2)
	{
		pv = apv.GetPtr(i);
		cMemory.Remove(pv);
	}

	AssertSize(0, cMemory.NumElements());
	AssertSize(8, cMemory.NumFreeLists());
	cMemory.Compact();
#ifdef BUILD_64
	AssertSize(32, cMemory.ByteSize());
#else
	AssertSize(16, cMemory.ByteSize());
#endif // BUILD_64
	AssertSize(0, cMemory.NumFreeLists());

	apv.Kill();

	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataMemoryIteration(void)
{
	CDataMemory			cMemory;
	void*				pv8192;
	void*				pv48;
	void*				pv64;
	void*				pv9036;
	SMemoryIterator		sIterator;
	SMemory				sResult;

	cMemory.Init(&gcDataMemoryFreeListParams);

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
void TestDataMemoryAfterParamsKilled(void)
{
	CMemoryFreeListParams	cParams;
	CDataMemory				cMemory;
	void*					pv;

	cParams.Init(sizeof(SDataMemoryAllocation), true);
	AssertTrue(cParams.GetMaxFreeListElementSize() > 0);
	AssertNotNull(cParams.GetFreeListParamsForSize(16));

	//Killed params are as params that were never initialised: no free lists, so everything is large.
	cParams.Kill();
	AssertSize(0, cParams.GetMaxFreeListElementSize());
	AssertNull(cParams.GetFreeListParamsForSize(16));

	cMemory.Init(&cParams);
	pv = cMemory.Add(16);
	AssertNotNull(pv);
	AssertSize(1, cMemory.NumElements());
	AssertTrue(cMemory.Remove(pv));
	AssertSize(0, cMemory.NumElements());
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A Debug build fills a freed allocation with MEMORY_FREED_FILL, all but
// its header's uiSize, to its last byte.  A second allocation in the same
// free list keeps the slab the freed one is in.
//
//////////////////////////////////////////////////////////////////////////
void TestDataMemoryRemoveFillsFreed(void)
{
#ifdef _DEBUG
	CDataMemory	cMemory;
	void*			pvKept;
	void*			pvFreed;
	uint8*			puiFreed;
	size			i;
	size			uiEnd;
	size			uiWrong;

	cMemory.Init(&gcDataMemoryFreeListParams);
	pvKept = cMemory.Add(24);
	pvFreed = cMemory.Add(24);
	memset(pvFreed, 0x12, 24);
	AssertTrue(cMemory.Remove(pvFreed));

	puiFreed = (uint8*)DATA_MEMORY_GET_ALLOCATION(pvFreed);
	AssertLSize(24, ((SDataMemoryAllocation*)puiFreed)->uiSize);
	uiEnd = sizeof(SDataMemoryAllocation) + 24;
	uiWrong = 0;
	for (i = sizeof(((SDataMemoryAllocation*)puiFreed)->uiSize); i < uiEnd; i++)
	{
		if (puiFreed[i] != MEMORY_FREED_FILL)
		{
			uiWrong++;
		}
	}
	AssertSize(0, uiWrong);

	AssertTrue(cMemory.Remove(pvKept));
	cMemory.Kill();
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDataMemory(void)
{
	BeginTests();
	WordsInit();
	DataMemoryInit();

	TestDataMemorySizeof();
	TestDataMemoryAdd();
	TestDataMemoryIteration();
	TestDataMemoryAfterParamsKilled();
	TestDataMemoryRemoveFillsFreed();

	DataMemoryKill();
	WordsKill();
	TestStatistics();
}

