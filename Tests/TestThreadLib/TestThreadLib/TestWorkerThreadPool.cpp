#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Timer.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "ThreadLib/AtomicInt.h"
#include "BaseLib/SafeArrayBlock.h"
#include "TestLib/Assert.h"


#define TEST_THREAD_INTEGER_COUNT 10000
#define TEST_THREAD_HALFWAY_WAIT_MILLISECONDS 10000


//Pushes its x half the times, waits until every pusher has pushed half (all of them are running at once), then pushes
//the rest: so every pusher's x is in the array before and after others', however the threads are scheduled.
class CPusher
{
private:
	int					mix;
	int					miThreadId;
	CSafeArrayBlock*	mpcArray;
	CAtomicInt			miRunning;
	CAtomicInt*			mpiHalfway;		//The pushers that have pushed half, shared by all of them...
	int					miPushers;		//...and how many there are.
	bool				mbTimedOut;		//The others weren't all halfway after TEST_THREAD_HALFWAY_WAIT_MILLISECONDS.

public:
	void Init(int x, CSafeArrayBlock* pcArray, CAtomicInt* piHalfway, int iPushers)
	{
		mix = x;
		mpcArray = pcArray;
		miThreadId = -1;
		miRunning.Set(1);
		mpiHalfway = piHalfway;
		miPushers = iPushers;
		mbTimedOut = false;
	}

	void Start(int iThreadId)
	{
		miThreadId = iThreadId;
	}

	void Run(void)
	{
		int		i;
		CTimer	cTimer;

		for (i = 0; i < TEST_THREAD_INTEGER_COUNT / 2; i++)
		{
			mpcArray->Add(&mix);
		}

		mpiHalfway->Increment();
		cTimer.Init();
		while (mpiHalfway->Get() != miPushers)
		{
			cTimer.Update();
			if (cTimer.GetTotalTimeInMillieconds() > TEST_THREAD_HALFWAY_WAIT_MILLISECONDS)
			{
				mbTimedOut = true;
				break;
			}
		}

		for (; i < TEST_THREAD_INTEGER_COUNT; i++)
		{
			mpcArray->Add(&mix);
		}
		miRunning.Set(0);
	}

	bool IsDone(void)
	{
		return miRunning.Get() == 0;
	}

	bool IsTimedOut(void)
	{
		return mbTimedOut;
	}
};


void RunPusher(int iThreadId, void* pvPusher)
{
	CPusher*	pcPusher;

	pcPusher = (CPusher*)pvPusher;
	pcPusher->Start(iThreadId);
	pcPusher->Run();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadPoolConcurrency(void)
{
	CSafeArrayBlock				aiArray;
	CArrayTemplate<CPusher>		acPushers;
	size						i;
	CPusher*					pcPusher;
	size						iNumHardwareTheads;
	size						iDoneCount;
	int							iPrevX;
	int							iX;
	size						iGreatherThanCount;
	CWorkerThreadPool			cPool;
	CAtomicInt					iHalfway;

	iNumHardwareTheads = NumHardwareThreads();
	acPushers.Init();
	aiArray.Init(sizeof(int));
	iHalfway.Set(0);

	for (i = 0; i < iNumHardwareTheads * 2; i++)
	{
		pcPusher = acPushers.Add();
		pcPusher->Init((int)i + 101, &aiArray, &iHalfway, (int)(iNumHardwareTheads * 2));
	}

	cPool.Init((int)(iNumHardwareTheads * 2));
	for (i = 0; i < iNumHardwareTheads * 2; i++)
	{
		pcPusher = acPushers.Get(i);
		cPool.Push(RunPusher, pcPusher);
	}

	iDoneCount = 0;
	while (iDoneCount != iNumHardwareTheads * 2)
	{
		iDoneCount = 0;
		for (i = 0; i < iNumHardwareTheads * 2; i++)
		{
			pcPusher = acPushers.Get(i);
			if (pcPusher->IsDone())
			{
				iDoneCount++;
			}
		}
	}

	AssertSize(iNumHardwareTheads * TEST_THREAD_INTEGER_COUNT * 2, aiArray.NumElements());
	for (i = 0; i < iNumHardwareTheads * 2; i++)
	{
		AssertFalse(acPushers.Get(i)->IsTimedOut());
	}
	Pass();

	iGreatherThanCount = 0;
	iPrevX = 0;
	for (i = 0; i < aiArray.NumElements(); i++)
	{
		aiArray.Get(i, &iX);
		if (iPrevX > iX)
		{
			iGreatherThanCount++;
		}
		iPrevX = iX;
	}

	AssertTrue(iGreatherThanCount > 0);
	AssertTrue(iGreatherThanCount < aiArray.NumElements());
	Pass();

	aiArray.Kill();
	acPushers.Kill();
	cPool.Stop(true);
	cPool.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadPoolInitialiseAndKill(void)
{
	CWorkerThreadPool		cPool;

	cPool.Init();
	cPool.Kill();
}

static CAtomicInt			giTestWorkerThreadsStarted;
static CAtomicInt			giTestWorkerThreadsEnded;
static CAtomicInt			giTestWorkerThreadsRanOutsideThem;
static thread_local bool	gbTestWorkerThreadStarted = false;	//The running thread is between its started and its ending.


static bool TestWorkerThreadStarted(void)
{
	gbTestWorkerThreadStarted = true;
	giTestWorkerThreadsStarted.Increment();
	return true;
}


static bool TestWorkerThreadEnding(void)
{
	gbTestWorkerThreadStarted = false;
	giTestWorkerThreadsEnded.Increment();
	return true;
}


static void TestWorkerThreadCountedTask(int iThreadId, void* pvCount)
{
	if (!gbTestWorkerThreadStarted)
	{
		giTestWorkerThreadsRanOutsideThem.Increment();
	}
	((CAtomicInt*)pvCount)->Increment();
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the pool's threads calls its started function once before anything it runs and its ending function once
// after everything.
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadPoolThreadFunctions(void)
{
	CWorkerThreadPool	cPool;
	CAtomicInt			iCount;
	int					i;

	giTestWorkerThreadsStarted.Set(0);
	giTestWorkerThreadsEnded.Set(0);
	giTestWorkerThreadsRanOutsideThem.Set(0);
	iCount.Set(0);

	cPool.Init(4, TestWorkerThreadStarted, TestWorkerThreadEnding);
	for (i = 0; i < 100; i++)
	{
		cPool.Push(TestWorkerThreadCountedTask, &iCount);
	}
	cPool.Stop(true);
	AssertInt(100, iCount.Get());
	AssertInt(4, giTestWorkerThreadsStarted.Get());
	AssertInt(4, giTestWorkerThreadsEnded.Get());
	AssertInt(0, giTestWorkerThreadsRanOutsideThem.Get());
	cPool.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadPool(void)
{
	BeginTests();
	MemoryInit();

	TestWorkerThreadPoolInitialiseAndKill();
	TestWorkerThreadPoolConcurrency();
	TestWorkerThreadPoolThreadFunctions();

	MemoryKill();
	TestStatistics();
}

