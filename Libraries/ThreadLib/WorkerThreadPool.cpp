#include "WorkerThreadPool.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RunWorkerThread(void* pvData)
{
	SWorkerThread*	psThread;

	psThread = (SWorkerThread*)pvData;
	psThread->pcPool->Work(psThread->iThreadId);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Init(void)
{
	Init(NumHardwareThreads());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Init(int iNumThreads)
{
	Init(iNumThreads, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Each thread calls fThreadStarted before the first function it runs and fThreadEnding after the last (either can be
// NULL).
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Init(int iNumThreads, WorkerThreadFunction fThreadStarted, WorkerThreadFunction fThreadEnding)
{
	size			i;
	SWorkerThread*	psThread;

	mcMutex.Init();
	mcWake.Init();
	mcTasks.Init();
	mfThreadStarted = fThreadStarted;
	mfThreadEnding = fThreadEnding;
	miNumIdle = 0;
	mbDone = false;
	mbStop = false;

	//The threads keep pointers to their elements so the array must not grow after this.
	macThreads.Init();
	macThreads.Resize(iNumThreads);
	for (i = 0; i < (size)iNumThreads; i++)
	{
		psThread = macThreads.Get(i);
		psThread->cThread.Init();
		psThread->pcPool = this;
		psThread->iThreadId = (int)i;
		psThread->cThread.Start(RunWorkerThread, psThread);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Kill(void)
{
	Stop(false);
	macThreads.Kill();
	mcTasks.Kill();
	mcWake.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Stop(bool bWaitForTasks)
{
	size			i;
	SWorkerThread*	psThread;

	mcMutex.Lock();
	if (bWaitForTasks)
	{
		if (mbDone || mbStop)
		{
			mcMutex.Unlock();
			return;
		}
		mbDone = true;
	}
	else
	{
		if (mbStop)
		{
			mcMutex.Unlock();
			return;
		}
		mbStop = true;
		ClearTasks();
	}
	mcWake.WakeAll();
	mcMutex.Unlock();

	for (i = 0; i < macThreads.NumElements(); i++)
	{
		psThread = macThreads.Get(i);
		psThread->cThread.Join();
		psThread->cThread.Kill();
	}
	macThreads.ReInit();

	//Functions pushed to a pool with no threads are never run.
	mcMutex.Lock();
	ClearTasks();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWorkerThreadPool::Push(WorkerFunction fFunction, void* pvData)
{
	SWorkerTask*	psTask;

	mcMutex.Lock();
	if (mbDone || mbStop)
	{
		mcMutex.Unlock();
		return false;
	}

	psTask = mcTasks.InsertAfterTail();
	psTask->fFunction = fFunction;
	psTask->pvData = pvData;

	mcWake.WakeOne();
	mcMutex.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWorkerThreadPool::Size(void) const
{
	return (int)macThreads.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWorkerThreadPool::NumIdleThreads(void)
{
	int		iNumIdle;

	mcMutex.Lock();
	iNumIdle = miNumIdle;
	mcMutex.Unlock();
	return iNumIdle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Work(int iThreadId)
{
	SWorkerTask		sTask;

	if (mfThreadStarted)
	{
		mfThreadStarted();
	}

	mcMutex.Lock();
	for (;;)
	{
		if (PopTask(&sTask))
		{
			mcMutex.Unlock();
			sTask.fFunction(iThreadId, sTask.pvData);
			mcMutex.Lock();
		}
		else if (mbStop || mbDone)
		{
			break;
		}
		else
		{
			miNumIdle++;
			mcWake.Wait(&mcMutex);
			miNumIdle--;
		}
	}
	mcMutex.Unlock();

	if (mfThreadEnding)
	{
		mfThreadEnding();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWorkerThreadPool::PopTask(SWorkerTask* psTask)
{
	SWorkerTask*	psHead;

	psHead = mcTasks.GetHead();
	if (psHead == NULL)
	{
		return false;
	}

	*psTask = *psHead;
	mcTasks.Remove(psHead);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::ClearTasks(void)
{
	SWorkerTask*	psHead;

	for (;;)
	{
		psHead = mcTasks.GetHead();
		if (psHead == NULL)
		{
			break;
		}
		mcTasks.Remove(psHead);
	}
}

