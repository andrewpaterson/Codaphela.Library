#ifndef __WORKER_THREAD_POOL_H__
#define __WORKER_THREAD_POOL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "BaseLib/NativeThread.h"


//iThreadId is the index of the pool thread running the function.
typedef void (*WorkerFunction)(int iThreadId, void* pvData);


struct SWorkerTask
{
	WorkerFunction	fFunction;
	void*			pvData;
};


class CWorkerThreadPool;
struct SWorkerThread
{
	CNativeThread		cThread;
	CWorkerThreadPool*	pcPool;
	int					iThreadId;
};


//What each of a pool's threads calls once, before the first function it runs and after the last: to set up and take
//down whatever a thread needs of its own (StandardLib's ObjectsThreadInit and ObjectsThreadKill, for threads that use
//objects).  What it returns is not the pool's business.
typedef bool (*WorkerThreadFunction)(void);


//A fixed number of threads running pushed functions in the order they were pushed.
class CWorkerThreadPool
{
protected:
	CMutex								mcMutex;
	CConditionVariable					mcWake;
	CArrayTemplate<SWorkerThread>		macThreads;
	CLinkedListTemplate<SWorkerTask>	mcTasks;
	WorkerThreadFunction				mfThreadStarted;	//NULL if there is nothing to call.
	WorkerThreadFunction				mfThreadEnding;
	int									miNumIdle;
	bool								mbDone;
	bool								mbStop;

public:
	void	Init(void);
	void	Init(int iNumThreads);
	void	Init(int iNumThreads, WorkerThreadFunction fThreadStarted, WorkerThreadFunction fThreadEnding);
	void	Kill(void);

	//Waits for the threads to finish.  If bWaitForTasks then every pushed
	//function is run first, otherwise functions that have not started are dropped.
	void	Stop(bool bWaitForTasks = false);

	bool	Push(WorkerFunction fFunction, void* pvData);

	int		Size(void) const;
	int		NumIdleThreads(void);

	void	Work(int iThreadId);

protected:
	bool	PopTask(SWorkerTask* psTask);
	void	ClearTasks(void);
};


#endif // __WORKER_THREAD_POOL_H__

