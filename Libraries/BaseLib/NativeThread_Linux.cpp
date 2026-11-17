#ifdef LINUX_GNU
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "LinuxError.h"
#include "Logger.h"
#include "NativeThread.h"


static_assert(sizeof(pthread_t) <= NATIVE_THREAD_HANDLE_SIZE, "NATIVE_THREAD_HANDLE_SIZE is too small for a pthread_t.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static pthread_t& ThreadHandle(uint8* paThread)
{
	return *(pthread_t*)paThread;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void* NativeThreadProc(void* pvStart)
{
	RunNativeThreadStart((SNativeThreadStart*)pvStart);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeThread::Init(void)
{
	ThreadHandle(maThread) = 0;
	mbStarted = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeThread::Kill(void)
{
	if (mbStarted)
	{
		gcLogger.Error2(__METHOD__, " Thread was not joined or detached.", NULL);
	}
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeThread::Start(NativeThreadFunction fFunction, void* pvData)
{
	int					iResult;
	SNativeThreadStart*	psStart;

	psStart = NewNativeThreadStart(fFunction, pvData);
	iResult = pthread_create(&ThreadHandle(maThread), NULL, NativeThreadProc, psStart);
	if (iResult != 0)
	{
		free(psStart);
		return gcLogger.Error2(__METHOD__, " pthread_create failed [", LinuxErrorCodeToString(iResult), "].", NULL);
	}

	mbStarted = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeThread::Join(void)
{
	if (mbStarted)
	{
		pthread_join(ThreadHandle(maThread), NULL);
		ThreadHandle(maThread) = 0;
		mbStarted = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeThread::Detach(void)
{
	if (mbStarted)
	{
		pthread_detach(ThreadHandle(maThread));
		ThreadHandle(maThread) = 0;
		mbStarted = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ThreadYield(void)
{
	sched_yield();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ThreadSleep(int iMilliseconds)
{
	struct timespec	sRemaining;

	sRemaining.tv_sec = iMilliseconds / 1000;
	sRemaining.tv_nsec = (long)(iMilliseconds % 1000) * 1000000L;
	while ((nanosleep(&sRemaining, &sRemaining) == -1) && (errno == EINTR))
	{
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int NumHardwareThreads(void)
{
	long	iProcessors;

	iProcessors = sysconf(_SC_NPROCESSORS_ONLN);
	if (iProcessors < 1)
	{
		return 1;
	}
	return (int)iProcessors;
}


//The running thread's id, asked of the kernel once: asking is a system call, and locks ask often.
static __thread int		giCurrentThreadId = 0;
static bool				gbCurrentThreadIdForkHandled = false;


//////////////////////////////////////////////////////////////////////////
//
// The one thread of a process just forked has a new id, not the one remembered by the thread it was copied from.
//
//////////////////////////////////////////////////////////////////////////
static void ForgetCurrentThreadId(void)
{
	giCurrentThreadId = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Never 0.
//
//////////////////////////////////////////////////////////////////////////
int CurrentThreadId(void)
{
	if (giCurrentThreadId == 0)
	{
		if (!gbCurrentThreadIdForkHandled)
		{
			gbCurrentThreadIdForkHandled = true;
			pthread_atfork(NULL, NULL, ForgetCurrentThreadId);
		}
		giCurrentThreadId = (int)syscall(SYS_gettid);
	}
	return giCurrentThreadId;
}


#endif // LINUX_GNU

