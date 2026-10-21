#ifdef LINUX_GNU
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include "InterProcessSignal.h"


//The futex is not FUTEX_PRIVATE_FLAG; it is in memory other processes map.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Create(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Connect(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessSignal::Close(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInterProcessSignal::Wait(int iSequence, int iTimeoutMilliseconds)
{
	struct timespec		sTimeout;
	struct timespec*	psTimeout;
	long				iResult;

	if (iTimeoutMilliseconds >= 0)
	{
		sTimeout.tv_sec = iTimeoutMilliseconds / 1000;
		sTimeout.tv_nsec = (long)(iTimeoutMilliseconds % 1000) * 1000000L;
		psTimeout = &sTimeout;
	}
	else
	{
		psTimeout = NULL;
	}

	//Returns at once with EAGAIN if a Wake changed the sequence since BeginWait.
	iResult = syscall(SYS_futex, (int*)&mpsShared->iSequence, FUTEX_WAIT, iSequence, psTimeout, NULL, 0);
	mpsShared->iWaiters.Decrement();
	return (iResult == 0) || (errno != ETIMEDOUT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInterProcessSignal::Wake(void)
{
	//Orders what the waker did before the read of the waiters; see BeginWait.
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	if (mpsShared->iWaiters.Get() == 0)
	{
		return;
	}
	mpsShared->iSequence.Increment();
	syscall(SYS_futex, (int*)&mpsShared->iSequence, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
}


#endif // LINUX_GNU
