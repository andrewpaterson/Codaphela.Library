#include "BaseLib/Mutex.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"


#define MUTEX_THREADS				4
#define MUTEX_INCREMENTS_PER_THREAD	100000


struct SMutexCounter
{
	CMutex	cMutex;
	int		iCount;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncrementMutexCounter(int iThreadId, void* pvCounter)
{
	SMutexCounter*	psCounter;
	int				i;
	int				iCount;

	psCounter = (SMutexCounter*)pvCounter;
	for (i = 0; i < MUTEX_INCREMENTS_PER_THREAD; i++)
	{
		//Read and write separately so an unlocked increment would lose counts.
		psCounter->cMutex.Lock();
		iCount = psCounter->iCount;
		psCounter->iCount = iCount + 1;
		psCounter->cMutex.Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMutexThreads(void)
{
	SMutexCounter		sCounter;
	CWorkerThreadPool	cPool;
	int					i;

	sCounter.cMutex.Init();
	sCounter.iCount = 0;

	cPool.Init(MUTEX_THREADS);
	for (i = 0; i < MUTEX_THREADS; i++)
	{
		cPool.Push(IncrementMutexCounter, &sCounter);
	}
	cPool.Stop(true);
	cPool.Kill();

	AssertInt(MUTEX_THREADS * MUTEX_INCREMENTS_PER_THREAD, sCounter.iCount);

	sCounter.cMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//	The single threaded tests of CMutex are in TestBaseLib.
//
//////////////////////////////////////////////////////////////////////////
void TestMutex(void)
{
	BeginTests();

	TestMutexThreads();

	TestStatistics();
}

