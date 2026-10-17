#ifndef __THREADS_DONE_H__
#define __THREADS_DONE_H__
#include "AtomicInt.h"
#include "ThreadStateNotifier.h"


class CThreadsDone : public CThreadStateNotifer
{
public:
	CAtomicInt	miThreadsStopped;

	void Init(void);
	void Wait(int iCount);

protected:
	void ThreadStateChanged(CThread* pcThread, EThreadState eState);
};


#endif // __THREADS_DONE_H__

