#ifndef __INTER_PROCESS_DONE_H__
#define __INTER_PROCESS_DONE_H__
#include "InterProcessCounter.h"


//Counts the processes that have finished.  The waiting process Starts it,
//each other process Opens it and calls Stop when it is done.
class CInterProcessDone : public CInterProcessCounter
{
public:
	void Start(void);
	bool IsDone(int iCount);
	bool Wait(int iCount, int iTimeoutMilliseconds = ATOMIC_WAIT_FOREVER);
	void Stop(void);
};


#endif // __INTER_PROCESS_DONE_H__

