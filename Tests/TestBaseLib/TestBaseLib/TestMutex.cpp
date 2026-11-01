#include "BaseLib/Mutex.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMutexLockUnlock(void)
{
	CMutex	cMutex;
	int		i;

	cMutex.Init();

	//A mutex can be locked again once it is unlocked; a failure here never returns.
	for (i = 0; i < 1000; i++)
	{
		cMutex.Lock();
		cMutex.Unlock();
	}
	Pass();

	cMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMutexIndependent(void)
{
	CMutex	cMutex1;
	CMutex	cMutex2;

	cMutex1.Init();
	cMutex2.Init();

	//Two mutexes are separate; holding one does not hold the other.
	cMutex1.Lock();
	cMutex2.Lock();
	cMutex2.Unlock();
	cMutex1.Unlock();
	Pass();

	cMutex2.Kill();
	cMutex1.Kill();

	//And a killed mutex can be initialised again.
	cMutex1.Init();
	cMutex1.Lock();
	cMutex1.Unlock();
	cMutex1.Kill();
	Pass();
}


//////////////////////////////////////////////////////////////////////////
//
//	The tests of CMutex used by more than one thread are in TestThreadLib.
//
//////////////////////////////////////////////////////////////////////////
void TestMutex(void)
{
	BeginTests();

	TestMutexLockUnlock();
	TestMutexIndependent();

	TestStatistics();
}

