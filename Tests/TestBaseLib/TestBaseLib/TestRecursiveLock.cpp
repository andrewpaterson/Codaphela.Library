#include "BaseLib/NativeThread.h"
#include "BaseLib/RecursiveLock.h"
#include "TestLib/Assert.h"


#define TEST_RECURSIVE_LOCK_THREADS		8
#define TEST_RECURSIVE_LOCK_STEPS		10000


//////////////////////////////////////////////////////////////////////////
//
// The thread that holds it takes it again, and holds it until it has unlocked it as many times.
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void TestRecursiveLockDepth(void)
{
	CRecursiveLock<LOCK>	cLock;

	cLock.Init();
	AssertFalse(cLock.IsLockedByThisThread());
	AssertInt(0, cLock.Depth());

	cLock.Lock();
	AssertTrue(cLock.IsLockedByThisThread());
	AssertInt(1, cLock.Depth());
	cLock.Lock();
	cLock.Lock();
	AssertInt(3, cLock.Depth());
	cLock.Unlock();
	AssertTrue(cLock.IsLockedByThisThread());
	AssertInt(2, cLock.Depth());
	cLock.Unlock();
	cLock.Unlock();
	AssertFalse(cLock.IsLockedByThisThread());
	AssertInt(0, cLock.Depth());

	cLock.Lock();
	AssertInt(1, cLock.Depth());
	cLock.Unlock();
	cLock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRecursiveLockThreadId(void)
{
	int		iThread;

	iThread = CurrentThreadId();
	AssertTrue(iThread != 0);
	AssertInt(iThread, CurrentThreadId());
}


//The threads of TestRecursiveLockThreads are started on the hosts only: the 65C64 runs one thread.
#ifndef __LIBC65__


template<class LOCK>
struct STestRecursiveLockShared
{
	CRecursiveLock<LOCK>	cLock;
	int64					iCount;			//Added to under the lock, in two steps another thread would come between.
	int32					iThreadIds;		//The sum of the threads' ids.
	int32					iWrongDepths;
	int32					iLockedByOthers;
};


template<class LOCK>
static void TestRecursiveLockThread(void* pvShared)
{
	STestRecursiveLockShared<LOCK>*		psShared;
	int64								iCount;
	size								i;

	psShared = (STestRecursiveLockShared<LOCK>*)pvShared;
	AtomicAdd(&psShared->iThreadIds, CurrentThreadId());
	for (i = 0; i < TEST_RECURSIVE_LOCK_STEPS; i++)
	{
		if (psShared->cLock.IsLockedByThisThread())
		{
			AtomicAdd(&psShared->iLockedByOthers, 1);
		}

		psShared->cLock.Lock();
		iCount = psShared->iCount;
		psShared->cLock.Lock();
		if (psShared->cLock.Depth() != 2)
		{
			psShared->iWrongDepths++;
		}
		if ((i & 1023) == 0)
		{
			ThreadYield();
		}
		psShared->iCount = iCount + 1;
		psShared->cLock.Unlock();
		psShared->cLock.Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only one thread is ever inside the lock, however deep it is in it; a thread never takes a lock another holds for
// its own; and each thread has an id of its own.
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void TestRecursiveLockThreads(void)
{
	STestRecursiveLockShared<LOCK>	sShared;
	CNativeThread					acThreads[TEST_RECURSIVE_LOCK_THREADS];
	size							i;

	sShared.cLock.Init();
	sShared.iCount = 0;
	sShared.iThreadIds = 0;
	sShared.iWrongDepths = 0;
	sShared.iLockedByOthers = 0;
	for (i = 0; i < TEST_RECURSIVE_LOCK_THREADS; i++)
	{
		acThreads[i].Init();
		acThreads[i].Start(TestRecursiveLockThread<LOCK>, &sShared);
	}
	for (i = 0; i < TEST_RECURSIVE_LOCK_THREADS; i++)
	{
		acThreads[i].Join();
		acThreads[i].Kill();
	}
	AssertLong((int64)TEST_RECURSIVE_LOCK_THREADS * TEST_RECURSIVE_LOCK_STEPS, sShared.iCount);
	AssertInt(0, sShared.iWrongDepths);
	AssertInt(0, sShared.iLockedByOthers);
	AssertFalse(sShared.cLock.IsLockedByThisThread());

	//Were every thread's id this thread's, the sum would be so many times this one's.
	AssertTrue(sShared.iThreadIds != TEST_RECURSIVE_LOCK_THREADS * CurrentThreadId());
	sShared.cLock.Kill();
}


#endif // !__LIBC65__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRecursiveLock(void)
{
	BeginTests();

	TestRecursiveLockThreadId();
	TestRecursiveLockDepth<CMutex>();
	TestRecursiveLockDepth<CSpinLock>();
#ifndef __LIBC65__
	TestRecursiveLockThreads<CMutex>();
	TestRecursiveLockThreads<CSpinLock>();
#endif // !__LIBC65__

	TestStatistics();
}

