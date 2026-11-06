#ifndef __TEST_THREAD_LIB_NAME_H__
#define __TEST_THREAD_LIB_NAME_H__
#include <stdio.h>
#ifdef LINUX_GNU
#include <unistd.h>
#else
#include <process.h>
#endif // LINUX_GNU


#define TEST_THREAD_LIB_NAME_SIZE	256


//A shared memory, mutex or channel name of this test run's own: szName with the process's ID after it.  The names are
//global (/dev/shm on Linux, the session's Local\ on Windows), so two TestThreadLib runs at once would otherwise share
//each other's queues and mutexes.  Forked processes are given the names, so only the test making them calls this.
inline char* TestThreadLibName(char* szDest, const char* szName)
{
#ifdef LINUX_GNU
	snprintf(szDest, TEST_THREAD_LIB_NAME_SIZE, "%s.%ld", szName, (long)getpid());
#else
	snprintf(szDest, TEST_THREAD_LIB_NAME_SIZE, "%s.%ld", szName, (long)_getpid());
#endif // LINUX_GNU
	return szDest;
}


#endif // __TEST_THREAD_LIB_NAME_H__

