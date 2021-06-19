#ifndef __THREAD_STATE_NOTIFIER_H__
#define __THREAD_STATE_NOTIFIER_H__
#include "ThreadState.h"

class CThread;
class CThreadStateNotifer
{
friend class CThread;
protected:
	virtual void ThreadStateChanged(CThread* pcThread, EThreadState eState) =0;
};

#endif // !__THREAD_STATE_NOTIFIER_H__

