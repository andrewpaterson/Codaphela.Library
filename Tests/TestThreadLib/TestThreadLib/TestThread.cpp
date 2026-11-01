#include "BaseLib/NativeThread.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/SafeArrayBlock.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"
#include "PoolProducerThread.h"
#include "PoolConsumerThread.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadStandAlone(void)
{
	CPoolProducerThread		cProducer1;
	CPoolProducerThread		cProducer2;
	CPoolProducerThread		cProducer3;
	CPoolProducerThread		cProducer4;
	CPoolProducerThread		cProducer5;

	CPoolConsumerThread		cConsumer1;
	CPoolConsumerThread		cConsumer2;

	int				iTotalResult;
	CSafeArrayBlock cQueue;
	
	cQueue.Init(sizeof(int));
	cProducer1.Init(&cQueue, 100, 200200);
	cProducer2.Init(&cQueue, 100, 1001);
	cProducer3.Init(&cQueue, 100, 1000000);
	cProducer4.Init(&cQueue, 100, 7);
	cProducer5.Init(&cQueue, 100, 13);
	cConsumer1.Init(&cQueue);
	cConsumer2.Init(&cQueue);

	cProducer1.Start();
	cProducer2.Start();
	cProducer3.Start();
	cProducer4.Start();
	cProducer5.Start();
	cConsumer1.Start();
	cConsumer2.Start();

	for (;;)
	{
		if (cProducer1.IsDone() &&
			cProducer2.IsDone() &&
			cProducer3.IsDone() &&
			cProducer4.IsDone() &&
			cProducer5.IsDone())
		{
			cConsumer1.TryStop();
			cConsumer2.TryStop();
		}

		if (cConsumer1.IsDone() &&
			cConsumer2.IsDone())
		{
			break;
		}
		ThreadSleep(16);
	}

	iTotalResult = cConsumer1.GetResult() + cConsumer2.GetResult();

	cProducer1.Kill();
	cProducer2.Kill();
	cProducer3.Kill();
	cProducer4.Kill();
	cProducer5.Kill();
	cConsumer1.Kill();
	cConsumer2.Kill();

	AssertInt(120122100, iTotalResult);

	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadInThreadPool(void)
{
	CSafeArrayBlock				cQueue;
	CWorkerThreadPool			cPool;

	NumHardwareThreads();
	cQueue.Init(sizeof(int));
	cPool.Init();


	//You should probably write this test.

	cPool.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThread(void)
{
	BeginTests();
	MemoryInit();

	TestThreadStandAlone();
	TestThreadInThreadPool();

	MemoryKill();
	TestStatistics();
}

