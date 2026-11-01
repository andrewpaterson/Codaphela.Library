#ifndef __SHARED_MEMORY_THREAD_H__
#define __SHARED_MEMORY_THREAD_H__
#include "ThreadLib/Thread.h"
#include "SharedMemoryFill.h"


class CSharedMemoryThread : public CThread
{
CONSTRUCTABLE(CSharedMemoryThread);
private:
	CSharedMemoryFill	mcFill;

public:
	CSharedMemoryThread(void) : CThread() {}
	CSharedMemoryThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify) : CThread(pcStarter, pcNotify) {}

	CSharedMemoryThread* Init(char* szSharedMemoryName, char* szMutexName, char* szFillChar, int iChunkSize)
	{
		CThread::Init();
		mcFill.Init(szSharedMemoryName, szMutexName, szFillChar, iChunkSize);
		return this;
	}


	virtual void Run(void)
	{
		mcFill.Run();
	}
};


#endif // !__SHARED_MEMORY_THREAD_H__

