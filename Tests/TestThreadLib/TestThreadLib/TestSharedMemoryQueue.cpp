#include "BaseLib/NativeThread.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Chars.h"
#include "BaseLib/StdRandom.h"
#include "ThreadLib/InterProcessMutex.h"
#include "ThreadLib/ProcessFork.h"
#include "ThreadLib/SharedMemoryQueue.h"
#include "ThreadLib/SharedMemory.h"
#include "ThreadLib/ThreadPool.h"
#include "ThreadLib/ThreadsDone.h"
#include "ThreadLib/InterProcessDone.h"
#include "ThreadLib/InterProcessName.h"
#include "TestLib/Assert.h"
#include "SharedMemoryQueueProducerThread.h"
#include "SharedMemoryQueueConsumerThread.h"
#include "TestSharedMemoryQueue.h"
#include "TestTimeout.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueBasic(void)
{
	CSharedMemoryQueue	cQueue;
	bool				bResult;
	CChars				sz;
	char				szData[4 KB];
	size					uiDataSize;
	size					uiOneElementCacheSize;
	size					uiTwoElementCacheSize;

#ifdef BUILD_64
	uiOneElementCacheSize = 44;
	uiTwoElementCacheSize = 108;
#else
	uiOneElementCacheSize = 32;
	uiTwoElementCacheSize = 84;
#endif // BUILD_64

	bResult = cQueue.Init("TestSharedMemoryQueueBasic", 20 + sizeof(SMemoryCacheDescriptor));
	AssertTrue(bResult);
	AssertTrue(cQueue.IsEmpty());
	AssertSize(uiOneElementCacheSize, cQueue.GetCacheSize());

	sz.Init('A', 19);
	bResult = cQueue.Push(sz.Text(), sz.Length() + 1);  sz.Kill();
	AssertTrue(bResult);
	AssertFalse(cQueue.IsEmpty());
	AssertSize(uiOneElementCacheSize, cQueue.GetCacheSize());
	Pass();

	bResult = cQueue.Pop(szData, &uiDataSize, 4 KB);
	AssertTrue(bResult);
	AssertSize(20, uiDataSize);
	AssertString("AAAAAAAAAAAAAAAAAAA", szData);
	AssertTrue(cQueue.IsEmpty());
	AssertSize(uiOneElementCacheSize, cQueue.GetCacheSize());
	Pass();

	bResult = cQueue.Pop(szData, &uiDataSize, 4 KB);
	AssertFalse(bResult);
	AssertSize(0, uiDataSize);
	AssertTrue(cQueue.IsEmpty());

	sz.Init('B', 19);
	bResult = cQueue.Push(sz.Text(), sz.Length() + 1);  sz.Kill();
	AssertTrue(bResult);
	AssertFalse(cQueue.IsEmpty());
	AssertSize(uiOneElementCacheSize, cQueue.GetCacheSize());
	cQueue.ValidateQueue();
	Pass();

	sz.Init('C', 19);
	bResult = cQueue.Push(sz.Text(), sz.Length() + 1);  sz.Kill();
	AssertTrue(bResult);
	AssertSize(uiTwoElementCacheSize, cQueue.GetCacheSize());
	Pass();

	bResult = cQueue.Pop(szData, &uiDataSize, 4 KB);
	AssertTrue(bResult);
	AssertSize(20, uiDataSize);
	AssertString("BBBBBBBBBBBBBBBBBBB", szData);
	AssertFalse(cQueue.IsEmpty());
	AssertSize(uiTwoElementCacheSize, cQueue.GetCacheSize());
	Pass();

	bResult = cQueue.Pop(szData, &uiDataSize, 4 KB);
	AssertTrue(bResult);
	AssertSize(20, uiDataSize);
	AssertString("CCCCCCCCCCCCCCCCCCC", szData);
	AssertTrue(cQueue.IsEmpty());
	AssertSize(uiTwoElementCacheSize, cQueue.GetCacheSize());
	Pass();

	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SQueueTestSequence::Init(void)
{
	iSize = 100;
	c = 33;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SQueueTestSequence::Next(void)
{
	iSize++;
	if (iSize == 256)
	{
		iSize = 100;
	}

	c++;
	if (c == 122)
	{
		c = 33;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FillQueueElement(SSharedMemoryQueueElement* psElement, int iIndex, SQueueTestSequence* psSequence)
{
	memset(psElement->sz, psSequence->c, psSequence->iSize);
	psElement->sz[psSequence->iSize] = '\0';
	psElement->sHeader.miIndex = iIndex;
	psElement->sHeader.muiSize = (unsigned int)(psSequence->iSize + 1 + sizeof(SSharedMemoryQueueElementHeader));
	psSequence->Next();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CreateExpectedString(CChars* psz, int iProduce)
{
	int					i;
	char				acProducerData[256];
	SQueueTestSequence	sSequence;

	sSequence.Init();
	for (i = 0; i < iProduce; i++)
	{
		memset(acProducerData, sSequence.c, sSequence.iSize);
		psz->Append((char*)acProducerData, sSequence.iSize)->AppendNewLine();
		sSequence.Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertQueueResult(int* aiTaken, char (*aszData)[257], int iProduce)
{
	int		i;
	CChars	szExpected;
	CChars	szActual;

	AssertInt(iProduce, aiTaken[0] + aiTaken[1] + aiTaken[2] + aiTaken[3]);

	szExpected.Init();
	CreateExpectedString(&szExpected, iProduce);

	szActual.Init();
	for (i = 0; i < iProduce; i++)
	{
		szActual.Append(aszData[i])->AppendNewLine();
	}
	AssertString(szExpected.Text(), szActual.Text());

	szActual.Kill();
	szExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueOne(void)
{
	CSharedMemoryQueue	cQueueProducer;
	CSharedMemoryQueue	acQueueConsumer[4];
	int					i;
	CRandom				cRandom;
	char				acProducerData[256];
	char				aacData[4][256];
	SQueueTestSequence		sSequence;
	size					uiSize;
	bool				bResult;
	int					iProduce;
	int					iTaken;
	CChars				szExpected;
	CChars				szActual;

	cQueueProducer.Init("TestSharedMemoryQueueOne", 0);
	for (i = 0; i < 4; i++)
	{
		acQueueConsumer[i].Init("TestSharedMemoryQueueOne");
	}

	sSequence.Init();
	iProduce = 1000;
	cRandom.Init(723);

	szExpected.Init();
	szActual.Init();
	CreateExpectedString(&szExpected, 1000);

	for (i = 0; i < 200; i++)
	{
		memset(acProducerData, sSequence.c, sSequence.iSize);
		cQueueProducer.Push(acProducerData, sSequence.iSize);
		iProduce--;
		sSequence.Next();
	}
	Pass();

	AssertFalse(cQueueProducer.IsEmpty());
	Pass();
	AssertFalse(acQueueConsumer[0].IsEmpty());
	AssertFalse(acQueueConsumer[1].IsEmpty());
	AssertFalse(acQueueConsumer[2].IsEmpty());
	AssertFalse(acQueueConsumer[3].IsEmpty());
	Pass();

	iTaken = 0;
	for (;;)
	{
		i = cRandom.Next(0, 4);
		if (i == 0)
		{
			if (iProduce > 0)
			{
				memset(acProducerData, sSequence.c, sSequence.iSize);
				cQueueProducer.Push(acProducerData, sSequence.iSize);
				iProduce--;
				sSequence.Next();
			}
		}
		else
		{
			bResult = acQueueConsumer[i - 1].Pop(aacData[i - 1], &uiSize, 256);

			if (bResult)
			{
				szActual.Append(aacData[i - 1], uiSize)->AppendNewLine();

				iTaken++;
				if (iTaken == 1000)
				{
					break;
				}
			}
		}
	}

	AssertString(szExpected.Text(), szActual.Text());
	Pass();
	szExpected.Kill();
	szActual.Kill();

	for (i = 0; i < 4; i++)
	{
		acQueueConsumer[i].Kill();
	}
	cQueueProducer.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueMultiThread(void)
{
	CSharedMemoryQueueProducerThread*	pcProducer;
	CSharedMemoryQueueConsumerThread*	pcConsumer1;
	CSharedMemoryQueueConsumerThread*	pcConsumer2;
	CSharedMemoryQueueConsumerThread*	pcConsumer3;
	CSharedMemoryQueueConsumerThread*	pcConsumer4;
	CThreadPool							cPool;
	CThreadsDone						cProducersDone;
	CThreadsDone						cConsumersDone;
	SThreadedSharedQueueResult*			psResult;
	char								szSharedMemoryName[] = { "TestSharedMemoryQueueMultiThread" };
	CSharedMemoryQueue					cQueue;

	psResult = (SThreadedSharedQueueResult*)malloc(sizeof(SThreadedSharedQueueResult));
	memset(psResult, 0, sizeof(SThreadedSharedQueueResult));

	cPool.Init();

	cProducersDone.Init();
	cConsumersDone.Init();

	cQueue.Init(szSharedMemoryName, 0);

	pcProducer = cPool.Add<CSharedMemoryQueueProducerThread>()->Init(szSharedMemoryName);
	pcProducer->AddNotifier(&cProducersDone);

	pcProducer->Start();

	pcConsumer1 = cPool.Add<CSharedMemoryQueueConsumerThread>()->Init(szSharedMemoryName, &cProducersDone, psResult, 0);
	pcConsumer1->AddNotifier(&cConsumersDone);
	pcConsumer2 = cPool.Add<CSharedMemoryQueueConsumerThread>()->Init(szSharedMemoryName, &cProducersDone, psResult, 1);
	pcConsumer2->AddNotifier(&cConsumersDone);
	pcConsumer3 = cPool.Add<CSharedMemoryQueueConsumerThread>()->Init(szSharedMemoryName, &cProducersDone, psResult, 2);
	pcConsumer3->AddNotifier(&cConsumersDone);
	pcConsumer4 = cPool.Add<CSharedMemoryQueueConsumerThread>()->Init(szSharedMemoryName, &cProducersDone, psResult, 3);
	pcConsumer4->AddNotifier(&cConsumersDone);

	pcConsumer1->Start();
	pcConsumer2->Start();
	pcConsumer3->Start();
	pcConsumer4->Start();


	AssertTrue(cProducersDone.Wait(1, TEST_TIMEOUT));
	AssertTrue(cConsumersDone.Wait(4, TEST_TIMEOUT));

	AssertQueueResult(psResult->aiTaken, psResult->aszData, THREADED_SHARED_QUEUE_PRODUCE_SIZE);

	cPool.Kill();

	cQueue.Kill();

	free(psResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueMultiThreadThisProducer(void)
{
	CSharedMemoryQueue					cQueue;
	CSharedMemoryQueueConsumerThread*	pcConsumer;
	CThreadPool							cPool;
	CThreadsDone						cProducersDone;
	CThreadsDone						cConsumersDone;
	SThreadedSharedQueueResult*			psResult;
	int									iProcess;
	SQueueTestSequence						sSequence;
	int									iProduce;
	SSharedMemoryQueueElement			sProducerData;
	char								szQueueName[] = "TestMultiThreadThisProducer";

	psResult = (SThreadedSharedQueueResult*)malloc(sizeof(SThreadedSharedQueueResult));
	memset(psResult, 0, sizeof(SThreadedSharedQueueResult));

	cPool.Init();

	cProducersDone.Init();
	cConsumersDone.Init();

	cQueue.Init(szQueueName, 0);

	iProcess = 0;

	sSequence.Init();
	for (iProduce = 0; iProduce < THREADED_SHARED_QUEUE_PRODUCE_SIZE; iProduce++)
	{
		FillQueueElement(&sProducerData, iProduce, &sSequence);
		cQueue.Push(&sProducerData, sProducerData.sHeader.muiSize);

		if ((iProduce % 1000 == 999) && (iProcess < 4))
		{
			pcConsumer = cPool.Add<CSharedMemoryQueueConsumerThread>()->Init(szQueueName, &cProducersDone, psResult, iProcess);
			pcConsumer->AddNotifier(&cConsumersDone);
			pcConsumer->Start();
			iProcess++;
		}
	}
	cProducersDone.miThreadsStopped.Increment();

	AssertTrue(cConsumersDone.Wait(4, TEST_TIMEOUT));

	AssertQueueResult(psResult->aiTaken, psResult->aszData, THREADED_SHARED_QUEUE_PRODUCE_SIZE);

	cQueue.Kill();
	cPool.Kill();

	free(psResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueMultiProcessForkDuring(void)
{
	CSharedMemoryQueue				cQueue;
	char							szQueueName[] = "TestSharedMemoryQueueMultiProcess";
	char							szSharedResultName[] = "TestSharedMemoryQueueMultiProcessResult";
	char							szDoneName[] = "TestSharedMemoryQueueMultiProcessDone";
	int								iProduce;
	SSharedMemoryQueueElement		sProducerData;
	SQueueTestSequence					sSequence;
	CSharedMemory					cResult;
	SProcessSharedQueueResult*		psResult;
	CInterProcessDone				cProcessesDone;
	int								iProcess;
	char							szProcess[16];

	cResult.Init(szSharedResultName);
	psResult = (SProcessSharedQueueResult*)cResult.Create(sizeof(SProcessSharedQueueResult));
	memset(psResult, 0, sizeof(SProcessSharedQueueResult));

	cProcessesDone.Init(szDoneName);
	cProcessesDone.Start();
	iProcess = 0;

	cQueue.Init(szQueueName, 0);

	sSequence.Init();
	for (iProduce = 0; iProduce < PROCESS_SHARED_QUEUE_PRODUCE_SIZE; iProduce++)
	{
		FillQueueElement(&sProducerData, iProduce, &sSequence);
		cQueue.Push(&sProducerData, sProducerData.sHeader.muiSize);

		if ((iProduce % 1000 == 999) && (iProcess < 4))
		{
			ForkProcess("--test-shared-queue", szQueueName, szSharedResultName, szDoneName, IntToString(szProcess, 16, iProcess), false);
			iProcess++;
		}
	}

	cProcessesDone.Stop();
	AssertTrue(cProcessesDone.Wait(5, TEST_TIMEOUT));

	cQueue.Kill();
	cProcessesDone.Kill();

	AssertQueueResult(psResult->aiTaken, psResult->aszData, PROCESS_SHARED_QUEUE_PRODUCE_SIZE);

	
	cResult.Close();
	cResult.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueMultiProcessForkEarly(void)
{
	CSharedMemoryQueue				cQueue;
	char							szQueueName[] = "TestSharedMemoryQueueMultiProcess";
	char							szSharedResultName[] = "TestSharedMemoryQueueMultiProcessResult";
	char							szDoneName[] = "TestSharedMemoryQueueMultiProcessDone";
	int								iProduce;
	SSharedMemoryQueueElement		sProducerData;
	SQueueTestSequence					sSequence;
	CSharedMemory					cResult;
	SProcessSharedQueueResult* psResult;
	CInterProcessDone				cProcessesDone;

	cResult.Init(szSharedResultName);
	psResult = (SProcessSharedQueueResult*)cResult.Create(sizeof(SProcessSharedQueueResult));
	memset(psResult, 0, sizeof(SProcessSharedQueueResult));

	cProcessesDone.Init(szDoneName);
	cProcessesDone.Start();

	cQueue.Init(szQueueName, 0);
	ForkProcess("--test-shared-queue", szQueueName, szSharedResultName, szDoneName, "0", false);
	ForkProcess("--test-shared-queue", szQueueName, szSharedResultName, szDoneName, "1", false);
	ForkProcess("--test-shared-queue", szQueueName, szSharedResultName, szDoneName, "2", false);
	ForkProcess("--test-shared-queue", szQueueName, szSharedResultName, szDoneName, "3", false);

	sSequence.Init();
	for (iProduce = 0; iProduce < PROCESS_SHARED_QUEUE_PRODUCE_SIZE; iProduce++)
	{
		FillQueueElement(&sProducerData, iProduce, &sSequence);
		cQueue.Push(&sProducerData, sProducerData.sHeader.muiSize);
	}

	cProcessesDone.Stop();
	AssertTrue(cProcessesDone.Wait(5, TEST_TIMEOUT));

	cQueue.Kill();
	cProcessesDone.Kill();

	AssertQueueResult(psResult->aiTaken, psResult->aszData, PROCESS_SHARED_QUEUE_PRODUCE_SIZE);


	cResult.Close();
	cResult.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueueLongNames(void)
{
	CChars				sz;
	CChars				szName;
	CSharedMemoryQueue	acQueue[2];
	bool				bResult;
	int					i;
	int					j;
	char*				pcData;
	size					uiSize;
	size					uiDataSize;

	//A name that fits is not changed.  A longer one keeps its start and ends
	//with a hash of all of it; the Java client shortens names the same way.
	ShortenInterProcessName(&sz, "Local\\SharedMemory:WiredSkunk.1003935-1.1:A", 63);
	AssertString("Local\\SharedMemory:WiredSkunk.1003935-1.1:A", sz.Text());
	sz.Kill();
	ShortenInterProcessName(&sz, "Local\\SharedMemory:WiredSkunkConnectTest1009196.1009196-12.1:B", 59);
	AssertString("Local\\SharedMemory:WiredSkunkConnectTest10#2e0481ef1ac956c8", sz.Text());
	sz.Kill();
	ShortenInterProcessName(&sz, "Local\\SharedMemory:WiredSkunkConnectTest1009196.1009196-12.1:A", 59);
	AssertString("Local\\SharedMemory:WiredSkunkConnectTest10#2e0484ef1ac95be1", sz.Text());
	sz.Kill();

	//Two queues whose names are too long and only differ at the end do not
	//share their memory, however many times it grows.
	for (i = 0; i < 2; i++)
	{
		szName.Init("TestSharedMemoryQueueLongNames");
		szName.Append('x', 50);
		szName.Append(i == 0 ? ":A" : ":B");
		bResult = acQueue[i].Init(szName.Text(), 0);
		szName.Kill();
		AssertTrue(bResult);
	}

	//Each element is twice the size of the one before so the memory is
	//replaced by a larger file many times (more than ten).
	pcData = (char*)malloc(1 MB);
	for (j = 0; j < 16; j++)
	{
		uiSize = (size_t)32 << j;
		for (i = 0; i < 2; i++)
		{
			memset(pcData, 'A' + i, uiSize);
			bResult = acQueue[i].Push(pcData, uiSize);
			AssertTrue(bResult);
		}
	}
	for (j = 0; j < 16; j++)
	{
		uiSize = (size_t)32 << j;
		for (i = 0; i < 2; i++)
		{
			bResult = acQueue[i].Pop(pcData, &uiDataSize, 1 MB);
			AssertTrue(bResult);
			AssertInt((int)uiSize, (int)uiDataSize);
			AssertChar('A' + i, pcData[0]);
			AssertChar('A' + i, pcData[uiDataSize - 1]);
		}
	}
	free(pcData);

	for (i = 0; i < 2; i++)
	{
		AssertTrue(acQueue[i].IsEmpty());
		acQueue[i].Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryQueue(void)
{
	BeginTests();

	TestSharedMemoryQueueBasic();
	TestSharedMemoryQueueLongNames();
	TestSharedMemoryQueueOne();
	TestSharedMemoryQueueMultiThread();
	TestSharedMemoryQueueMultiThreadThisProducer();
	TestSharedMemoryQueueMultiProcessForkDuring();
	TestSharedMemoryQueueMultiProcessForkEarly();

	TestStatistics();
}

