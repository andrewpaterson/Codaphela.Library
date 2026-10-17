#include <stdlib.h>
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
	mpsHead = NULL;
	mpsTail = NULL;
	miNumIdle = 0;
	mbDone = false;
	mbStop = false;

	masThreads = (SWorkerThread*)malloc(iNumThreads * sizeof(SWorkerThread));
	miNumThreads = iNumThreads;
	for (i = 0; i < iNumThreads; i++)
	{
		psThread = &masThreads[i];
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
	mcWake.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::Stop(bool bWaitForTasks)
{
	int		i;

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

	for (i = 0; i < miNumThreads; i++)
	{
		masThreads[i].cThread.Join();
		masThreads[i].cThread.Kill();
	}
	free(masThreads);
	masThreads = NULL;
	miNumThreads = 0;

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

	psTask = (SWorkerTask*)malloc(sizeof(SWorkerTask));
	psTask->fFunction = fFunction;
	psTask->pvData = pvData;
	psTask->psNext = NULL;

	mcMutex.Lock();
	if (mbDone || mbStop)
	{
		mcMutex.Unlock();
		free(psTask);
		return false;
	}

	if (mpsTail)
	{
		mpsTail->psNext = psTask;
	}
	else
	{
		mpsHead = psTask;
	}
	mpsTail = psTask;

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
	return miNumThreads;
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
	SWorkerTask*	psTask;

	mcMutex.Lock();
	for (;;)
	{
		psTask = PopTask();
		if (psTask)
		{
			mcMutex.Unlock();
			psTask->fFunction(iThreadId, psTask->pvData);
			free(psTask);
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
SWorkerTask* CWorkerThreadPool::PopTask(void)
{
	SWorkerTask*	psTask;

	psTask = mpsHead;
	if (psTask)
	{
		mpsHead = psTask->psNext;
		if (mpsHead == NULL)
		{
			mpsTail = NULL;
		}
	}
	return psTask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadPool::ClearTasks(void)
{
	SWorkerTask*	psTask;

	for (;;)
	{
		psTask = PopTask();
		if (psTask == NULL)
		{
			break;
		}
		free(psTask);
	}
}

