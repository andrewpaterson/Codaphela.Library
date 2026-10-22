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
	int				i;
	SWorkerThread*	psThread;

	mcMutex.Init();
	mcWake.Init();
	mcTasks.Init();
	miNumIdle = 0;
	mbDone = false;
	mbStop = false;

	//The threads keep pointers to their elements so the array must not grow after this.
	macThreads.Init();
	macThreads.Resize(iNumThreads);
	for (i = 0; i < iNumThreads; i++)
	{
		psThread = macThreads.Get(i);
		psThread->cThread.Init();
		psThread->pcPool = this;
		psThread->iThreadId = i;
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
int CWorkerThreadPool::Size(void)
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

