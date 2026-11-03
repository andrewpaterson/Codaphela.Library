#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeArrayBlock.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"


#define SAFE_ARRAY_THREADS			4
#define SAFE_ARRAY_ADDS_PER_THREAD	10000


//A pool thread can run more than one job so each job carries its own index.
struct SSafeArrayJob
{
	CSafeArrayBlock*	pcArray;
	int					iJob;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddToSafeArrayBlock(int iThreadId, void* pvJob)
{
	SSafeArrayJob*	psJob;
	int				i;

	psJob = (SSafeArrayJob*)pvJob;
	for (i = 0; i < SAFE_ARRAY_ADDS_PER_THREAD; i++)
	{
		psJob->pcArray->Add(&psJob->iJob);
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
	SSafeArrayJob		asJobs[SAFE_ARRAY_THREADS];
	int					aiCount[SAFE_ARRAY_THREADS];
	int					i;
	int					iValue;
	bool				bValid;

	cArray.Init(sizeof(int));

	//Every thread adds at the same time; none of the adds may be lost.
	cPool.Init(SAFE_ARRAY_THREADS);
	for (i = 0; i < SAFE_ARRAY_THREADS; i++)
	{
		asJobs[i].pcArray = &cArray;
		asJobs[i].iJob = i;
		cPool.Push(AddToSafeArrayBlock, &asJobs[i]);
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

