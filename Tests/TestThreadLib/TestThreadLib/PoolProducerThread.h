#ifndef __POOL_PRODUCER_THREAD_H__
#define __POOL_PRODUCER_THREAD_H__
#include "ThreadLib/Thread.h"
#include "BaseLib/SafeArrayBlock.h"


class CPoolProducerThread : public CThread
{
CONSTRUCTABLE(CPoolProducerThread);
private:
	CSafeArrayBlock*	mpcQueue;
	int					miProductionCount;
	int					miProductionValue;

public:
	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CPoolProducerThread(void) : CThread() {}
	CPoolProducerThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify) : CThread(pcStarter, pcNotify) {}


	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	CPoolProducerThread* Init(CSafeArrayBlock* pcQueue, int iProductionCount, int iProductionValue)
	{
		CThread::Init();
		mpcQueue = pcQueue;
		miProductionCount = iProductionCount;
		miProductionValue = iProductionValue;
		return this;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////
	virtual void Run(void)
	{
		int i;
		for (i = 0; i < miProductionCount && IsRunning(); i++)
		{
			mpcQueue->Push(&miProductionValue);
		}
		i = 0;
	}
};


#endif // !__POOL_PRODUCER_THREAD_H__

