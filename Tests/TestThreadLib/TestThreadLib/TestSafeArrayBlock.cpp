#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeArrayBlock.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"


#define SAFE_ARRAY_THREADS			4
#define SAFE_ARRAY_ADDS_PER_THREAD	10000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddToSafeArrayBlock(int iThreadId, void* pvArray)
{
	CSafeArrayBlock*	pcArray;
	int					i;

	pcArray = (CSafeArrayBlock*)pvArray;
	for (i = 0; i < SAFE_ARRAY_ADDS_PER_THREAD; i++)
	{
		pcArray->Add(&iThreadId);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlockThreads(void)
{
	CSafeArrayBlock		cArray;
	CWorkerThreadPool	cPool;
	int					aiCount[SAFE_ARRAY_THREADS];
	int					i;
	int					iValue;
	bool				bValid;

	cArray.Init(sizeof(int));

	//Every thread adds at the same time; none of the adds may be lost.
	cPool.Init(SAFE_ARRAY_THREADS);
	for (i = 0; i < SAFE_ARRAY_THREADS; i++)
	{
		cPool.Push(AddToSafeArrayBlock, &cArray);
	}
	cPool.Stop(true);
	cPool.Kill();

	AssertSize(SAFE_ARRAY_THREADS * SAFE_ARRAY_ADDS_PER_THREAD, cArray.NumElements());

	memset(aiCount, 0, sizeof(aiCount));
	bValid = true;
	for (i = 0; i < (int)cArray.NumElements(); i++)
	{
		cArray.Get(i, &iValue);
		if ((iValue < 0) || (iValue >= SAFE_ARRAY_THREADS))
		{
			bValid = false;
			break;
		}
		aiCount[iValue]++;
	}
	AssertTrue(bValid);
	for (i = 0; i < SAFE_ARRAY_THREADS; i++)
	{
		AssertInt(SAFE_ARRAY_ADDS_PER_THREAD, aiCount[i]);
	}

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//	The single threaded tests of CSafeArrayBlock are in TestBaseLib.
//
//////////////////////////////////////////////////////////////////////////
void TestSafeArrayBlock(void)
{
	BeginTests();
	MemoryInit();

	TestSafeArrayBlockThreads();

	MemoryKill();
	TestStatistics();
}

