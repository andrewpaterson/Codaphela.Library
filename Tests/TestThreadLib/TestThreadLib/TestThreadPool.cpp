#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LogString.h"
#include "ThreadLib/ThreadPool.h"
#include "ThreadLib/ThreadsDone.h"
#include "ThreadLib/AtomicInt.h"
#include "BaseLib/SafeArrayBlock.h"
#include "TestLib/Assert.h"
#include "TestTimeout.h"
#include "PoolProducerThread.h"
#include "PoolConsumerThread.h"


class CConsumerTotal : public CThreadStateNotifer
{
public:
	CAtomicInt	miTotal;

	void Init(void)
	{
		miTotal.Set(0);
	}

	void ThreadStateChanged(CThread* pcThread, EThreadState eState)
	{
		CPoolConsumerThread* pcConsumer;

		if (eState == TS_Stopped)
		{
			pcConsumer = (CPoolConsumerThread*)pcThread;
			miTotal.Add(pcConsumer->GetResult());
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadPoolThreadDestroyed(void)
{
	CThreadPool				cPool;
	CPoolProducerThread*	pcProducer;
	CPoolConsumerThread*	pcConsumer;
	CSafeArrayBlock			cQueue;
	size					i;
	CArrayVoidPtr			apcConsumers;
	CThreadsDone			cProducersDone;
	CThreadsDone			cConsumersDone;
	CConsumerTotal			cTotal;

	cQueue.Init(sizeof(int));
	cPool.Init();

	apcConsumers.Init();
	cProducersDone.Init();
	cConsumersDone.Init();
	cTotal.Init();

	for (i = 0; i < 32; i++)
	{
		pcProducer = cPool.Add<CPoolProducerThread>()->Init(&cQueue, 1000, 1);
		pcProducer->AddNotifier(&cProducersDone);
		pcProducer->Start();
	}
	pcProducer = NULL;

	for (i = 0; i < 3; i++)
	{
		pcConsumer = cPool.Add<CPoolConsumerThread>()->Init(&cQueue);
		pcConsumer->AddNotifier(&cConsumersDone);
		pcConsumer->AddNotifier(&cTotal);
		apcConsumers.Add(pcConsumer);
		pcConsumer->Start();
	}
	pcConsumer = NULL;

	AssertTrue(cProducersDone.Wait(32, TEST_TIMEOUT));

	for (i = 0; i < apcConsumers.NumElements(); i++)
	{
		pcConsumer = (CPoolConsumerThread*)apcConsumers.GetPtr(i);
		pcConsumer->TryStop();
	}

	AssertTrue(cConsumersDone.Wait(3, TEST_TIMEOUT));

	AssertInt(32000, cTotal.miTotal.Get());

	apcConsumers.Kill();
	cPool.Kill();
	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadPool(void)
{
	BeginTests();
	MemoryInit();

	TestThreadPoolThreadDestroyed();

	MemoryKill();
	TestStatistics();
}

