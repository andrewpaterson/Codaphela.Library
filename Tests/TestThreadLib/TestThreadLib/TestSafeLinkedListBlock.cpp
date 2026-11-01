#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeLinkedListBlock.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"


#define SAFE_LIST_THREADS			4
#define SAFE_LIST_ADDS_PER_THREAD	10000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddToSafeLinkedListBlock(int iThreadId, void* pvList)
{
	CSafeLinkedListBlock*	pcList;
	int*					pi;
	int						i;

	pcList = (CSafeLinkedListBlock*)pvList;
	for (i = 0; i < SAFE_LIST_ADDS_PER_THREAD; i++)
	{
		//Only this thread uses the new node so it can be set after the list is unlocked.
		pi = (int*)pcList->InsertAfterTail(sizeof(int));
		*pi = iThreadId;
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
		cPool.Push(AddToSafeLinkedListBlock, &cList);
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

