#include "BaseLib/NativeThread.h"
#include "BaseLib/SpinLock.h"
#include "TestLib/Assert.h"


#define TEST_SPIN_LOCK_THREADS	8
#define TEST_SPIN_LOCK_STEPS	20000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSpinLockLockUnlock(void)
{
	CSpinLock	cLock;
	CSpinLock	cOther;

	cLock.Init();
	cOther.Init();

	cLock.Lock();
	AssertFalse(cLock.TryLock());
	AssertTrue(cOther.TryLock());
	cOther.Unlock();
	cLock.Unlock();

	AssertTrue(cLock.TryLock());
	AssertFalse(cLock.TryLock());
	cLock.Unlock();
	cLock.Lock();
	cLock.Unlock();

	cOther.Kill();
	cLock.Kill();
}


//The threads of TestSpinLockThreads are started on the hosts only: the 65C64 runs one thread.
#ifndef __LIBC65__


struct STestSpinLockShared
{
	CSpinLock	cLock;
	int64		iCount;		//Added to under the lock, in two steps another thread would come between.
};


static void TestSpinLockThread(void* pvShared)
{
	STestSpinLockShared*	psShared;
	int64					iCount;
	size					i;

	psShared = (STestSpinLockShared*)pvShared;
	for (i = 0; i < TEST_SPIN_LOCK_STEPS; i++)
	{
		psShared->cLock.Lock();
		iCount = psShared->iCount;
		if ((i & 1023) == 0)
		{
			//The thread holding the lock is switched out now and then: the others wait, and yield.
			ThreadYield();
		}
		psShared->iCount = iCount + 1;
		psShared->cLock.Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only one thread is ever inside the lock: no addition is lost.
//
//////////////////////////////////////////////////////////////////////////
void TestSpinLockThreads(void)
{
	STestSpinLockShared		sShared;
	CNativeThread			acThreads[TEST_SPIN_LOCK_THREADS];
	size					i;

	sShared.cLock.Init();
	sShared.iCount = 0;
	for (i = 0; i < TEST_SPIN_LOCK_THREADS; i++)
	{
		acThreads[i].Init();
		acThreads[i].Start(TestSpinLockThread, &sShared);
	}
	for (i = 0; i < TEST_SPIN_LOCK_THREADS; i++)
	{
		acThreads[i].Join();
		acThreads[i].Kill();
	}
	AssertLong((int64)TEST_SPIN_LOCK_THREADS * TEST_SPIN_LOCK_STEPS, sShared.iCount);
	AssertTrue(sShared.cLock.TryLock());
	sShared.cLock.Kill();
}


#endif // !__LIBC65__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSpinLock(void)
{
	BeginTests();

	TestSpinLockLockUnlock();
#ifndef __LIBC65__
	TestSpinLockThreads();
#endif // !__LIBC65__

	TestStatistics();
}

