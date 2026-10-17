#ifndef __WORKER_THREAD_POOL_H__
#define __WORKER_THREAD_POOL_H__
#include "Mutex.h"
#include "ConditionVariable.h"
#include "NativeThread.h"


//iThreadId is the index of the pool thread running the function.
typedef void (*WorkerFunction)(int iThreadId, void* pvData);


struct SWorkerTask
{
	WorkerFunction	fFunction;
	void*			pvData;
	SWorkerTask*	psNext;
};


class CWorkerThreadPool;
struct SWorkerThread
{
	CNativeThread		cThread;
	CWorkerThreadPool*	pcPool;
	int					iThreadId;
};


//A fixed number of threads running pushed functions in the order they were pushed.
class CWorkerThreadPool
{
protected:
	CMutex				mcMutex;
	CConditionVariable	mcWake;
	SWorkerThread*		masThreads;
	int					miNumThreads;
	SWorkerTask*		mpsHead;
	SWorkerTask*		mpsTail;
	int					miNumIdle;
	bool				mbDone;
	bool				mbStop;

public:
	void	Init(void);
	void	Init(int iNumThreads);
	void	Kill(void);

	//Waits for the threads to finish.  If bWaitForTasks then every pushed
	//function is run first, otherwise functions that have not started are dropped.
	void	Stop(bool bWaitForTasks = false);

	bool	Push(WorkerFunction fFunction, void* pvData);

	int		Size(void);
	int		NumIdleThreads(void);

	void	Work(int iThreadId);

protected:
	SWorkerTask*	PopTask(void);
	void			ClearTasks(void);
};


#endif // __WORKER_THREAD_POOL_H__

