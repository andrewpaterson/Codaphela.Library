#ifndef __SHARED_MEMORY_QUEUE_PRODUCER_THREAD_H__
#define __SHARED_MEMORY_QUEUE_PRODUCER_THREAD_H__
#include "ThreadLib/SharedMemoryQueue.h"
#include "ThreadLib/Thread.h"
#include "SharedMemoryQueueTheadElement.h"
#include "TestSharedMemoryQueue.h"
#include "SharedMemoryQueueConsumerThread.h"


class CSharedMemoryQueueProducerThread : public CThread
{
CONSTRUCTABLE(CSharedMemoryQueueProducerThread);
private:
	CChars				mszName;

public:
	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CSharedMemoryQueueProducerThread(void) : CThread() {}
	CSharedMemoryQueueProducerThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify) : CThread(pcStarter, pcNotify) {}

	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CSharedMemoryQueueProducerThread* Init(char* szName)
	{
		CThread::Init();
		mszName.Init(szName);
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
		int								iProduce;
		SSharedMemoryQueueElement		sProducerData;
		SQueueTestSequence				sSequence;
		CSharedMemoryQueue				cQueue;
		bool							bResult;

		bResult = cQueue.Init(mszName.Text());
		if (!bResult)
		{
			return;
		}

		sSequence.Init();
		for (iProduce = 0; iProduce < THREADED_SHARED_QUEUE_PRODUCE_SIZE; iProduce++)
		{
			FillQueueElement(&sProducerData, iProduce, &sSequence);
			cQueue.Push(&sProducerData, sProducerData.sHeader.muiSize);
		}

		cQueue.Kill();
	}
};


#endif // !__SHARED_MEMORY_QUEUE_PRODUCER_THREAD_H__

