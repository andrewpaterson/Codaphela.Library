#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeLinkedListBlock.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"


#define SAFE_LIST_THREADS			4
#define SAFE_LIST_ADDS_PER_THREAD	10000


//A pool thread can run more than one job so each job carries its own index.
struct SSafeListJob
{
	CSafeLinkedListBlock*	pcList;
	int						iJob;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddToSafeLinkedListBlock(int iThreadId, void* pvJob)
{
	SSafeListJob*	psJob;
	int*			pi;
	int				i;

	psJob = (SSafeListJob*)pvJob;
	for (i = 0; i < SAFE_LIST_ADDS_PER_THREAD; i++)
	{
		//Only this thread uses the new node so it can be set after the list is unlocked.
		pi = (int*)psJob->pcList->InsertAfterTail(sizeof(int));
		*pi = psJob->iJob;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSafeLinkedListBlockThreads(void)
{
	CSafeLinkedListBlock	cList;
	CWorkerThreadPool		cPool;
	SSafeListJob			asJobs[SAFE_LIST_THREADS];
	int						aiCount[SAFE_LIST_THREADS];
	int						i;
	int*					pi;
	int						iNodes;
	bool					bValid;

	cList.Init();

	//Every thread inserts at the same time; none of the nodes may be lost.
	cPool.Init(SAFE_LIST_THREADS);
	for (i = 0; i < SAFE_LIST_THREADS; i++)
	{
		asJobs[i].pcList = &cList;
		asJobs[i].iJob = i;
		cPool.Push(AddToSafeLinkedListBlock, &asJobs[i]);
	}
	cPool.Stop(true);
	cPool.Kill();

	AssertInt(SAFE_LIST_THREADS * SAFE_LIST_ADDS_PER_THREAD, cList.NumElements());

	memset(aiCount, 0, sizeof(aiCount));
	bValid = true;
	iNodes = 0;
	pi = (int*)cList.GetHead();
	while (pi)
	{
		if ((*pi < 0) || (*pi >= SAFE_LIST_THREADS))
		{
			bValid = false;
			break;
		}
		aiCount[*pi]++;
		iNodes++;
		pi = (int*)cList.GetNext(pi);
	}
	AssertTrue(bValid);
	AssertInt(SAFE_LIST_THREADS * SAFE_LIST_ADDS_PER_THREAD, iNodes);
	for (i = 0; i < SAFE_LIST_THREADS; i++)
	{
		AssertInt(SAFE_LIST_ADDS_PER_THREAD, aiCount[i]);
	}

	cList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//	The single threaded tests of CSafeLinkedListBlock are in TestBaseLib.
//
//////////////////////////////////////////////////////////////////////////
void TestSafeLinkedListBlock(void)
{
	BeginTests();
	MemoryInit();

	TestSafeLinkedListBlockThreads();

	MemoryKill();
	TestStatistics();
}

