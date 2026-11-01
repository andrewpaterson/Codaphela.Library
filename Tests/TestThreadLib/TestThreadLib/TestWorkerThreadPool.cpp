#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LogString.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "ThreadLib/AtomicInt.h"
#include "BaseLib/SafeArrayBlock.h"
#include "TestLib/Assert.h"


#define TEST_THREAD_INTEGER_COUNT 10000


class CPusher
{
private:
	int					mix;
	int					miThreadId;
	CSafeArrayBlock*	mpcArray;
	CAtomicInt			miRunning;

public:
	void Init(int x, CSafeArrayBlock* pcArray)
	{
		mix = x;
		mpcArray = pcArray;
		miThreadId = -1;
		miRunning.Set(1);
	}

	void Start(int iThreadId)
	{
		miThreadId = iThreadId;
	}

	void Run(void)
	{
		int i;

		for (i = 0; i < TEST_THREAD_INTEGER_COUNT; i++)
		{
			mpcArray->Add(&mix);
		}
		miRunning.Set(0);
	}

	bool IsDone(void)
	{
		return miRunning.Get() == 0;
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

	iNumHardwareTheads = NumHardwareThreads();
	acPushers.Init();
	aiArray.Init(sizeof(int));

	for (i = 0; i < iNumHardwareTheads * 2; i++)
	{
		pcPusher = acPushers.Add();
		pcPusher->Init((int)i + 101, &aiArray);
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

	MemoryKill();
	TestStatistics();
}

