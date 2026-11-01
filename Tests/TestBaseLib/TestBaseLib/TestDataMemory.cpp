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
void TestDataMemory(void)
{
	BeginTests();
	WordsInit();
	DataMemoryInit();

	TestDataMemorySizeof();
	TestDataMemoryAdd();
	TestDataMemoryIteration();

	DataMemoryKill();
	WordsKill();
	TestStatistics();
}

