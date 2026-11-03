#ifndef __SHARED_MEMORY_QUEUE_CONSUMER_THREAD_H__
#define __SHARED_MEMORY_QUEUE_CONSUMER_THREAD_H__
#include "BaseLib/NativeThread.h"
#include "ThreadLib/SharedMemoryQueue.h"
#include "ThreadLib/Thread.h"
#include "SharedMemoryQueueTheadElement.h"
#include "TestSharedMemoryQueue.h"


class CSharedMemoryQueueConsumerThread : public CThread
{
CONSTRUCTABLE(CSharedMemoryQueueConsumerThread);
private:
	CThreadsDone*				mpcProducersDone;
	SThreadedSharedQueueResult*	mpsResult;
	int							miProcessNum;
	CChars						mszName;

public:
	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CSharedMemoryQueueConsumerThread(void) : CThread() {}
	CSharedMemoryQueueConsumerThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify) : CThread(pcStarter, pcNotify) {}


	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CSharedMemoryQueueConsumerThread* Init(char* szName, CThreadsDone* pcProducersDone, SThreadedSharedQueueResult* psResult, int iProcessNum)
	{
		mpcProducersDone = pcProducersDone;
		CThread::Init();
		mszName.Init(szName);
		mpsResult = psResult;
		miProcessNum = iProcessNum;
		return this;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	void Kill()
	{
		mszName.Kill();
		CThread::Kill();
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	void Run(void)
	{
		bool						bResult;
		bool						bDone;
		SSharedMemoryQueueElement	sConsumerData;
		size							uiSize;
		int							iTaken;
		CSharedMemoryQueue			cQueue;

		bResult = cQueue.Init(mszName.Text());
		if (!bResult)
		{
			return;
		}

		iTaken = 0;
		do
		{
			//Done is read before the pop: a pop that fails after the producers are done means the queue is empty.  Read
			//after, a producer could push its last elements while this thread yields.
			bDone = mpcProducersDone->miThreadsStopped.Get() >= 1;
			bResult = cQueue.Pop(&sConsumerData, &uiSize, sizeof(SSharedMemoryQueueElement));
			if (bResult)
			{
				iTaken++;
				strcpy(mpsResult->aszData[sConsumerData.sHeader.miIndex], sConsumerData.sz);
			}
			else
			{
				ThreadYield();
			}
		} while (!bDone || bResult);
		mpsResult->aiTaken[miProcessNum] = iTaken;

		cQueue.Kill();
	}
};


#endif // !__SHARED_MEMORY_QUEUE_CONSUMER_THREAD_H__

