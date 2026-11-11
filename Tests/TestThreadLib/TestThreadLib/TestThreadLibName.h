#ifndef __TEST_THREAD_LIB_NAME_H__
#define __TEST_THREAD_LIB_NAME_H__
#include <stdio.h>
#ifdef LINUX_GNU
#include <unistd.h>
#else
#include <process.h>
#endif // LINUX_GNU


#define TEST_THREAD_LIB_NAME_SIZE	256


//A shared memory, mutex or channel name of this test run's own: szName with the process's ID and a number counting the
//names made after it.  The names are global (/dev/shm on Linux, the session's Local\ on Windows), so two TestThreadLib
//runs at once would otherwise share each other's queues and mutexes.  The number keeps a test that is repeated, or one
//test after another with the same name, from creating a name a forked process of the one before still has open: a
//process signals it's done before it lets go, and the name can't be created again until the last one does.  Forked
//processes are given the names, so only the test making them calls this.
inline char* TestThreadLibName(char* szDest, const char* szName)
{
	static int	iNames = 0;

	iNames++;
#ifdef LINUX_GNU
	snprintf(szDest, TEST_THREAD_LIB_NAME_SIZE, "%s.%ld.%d", szName, (long)getpid(), iNames);
#else
	snprintf(szDest, TEST_THREAD_LIB_NAME_SIZE, "%s.%ld.%d", szName, (long)_getpid(), iNames);
#endif // LINUX_GNU
	return szDest;
}


#endif // __TEST_THREAD_LIB_NAME_H__

