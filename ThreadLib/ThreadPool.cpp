#include "BaseLib/ConstructorCall.h"
#include "ThreadPool.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThreadPool::Init(void)
{
	mcThreads.Init();
	mcPool.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThreadPool::Kill(void)
{
	mcPool.Kill();
	mcThreads.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RunThread(int iThreadId, CThread* pcThread)
{
	pcThread->Start(iThreadId, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThreadPool::StartThread(CThread* pcThread)
{
	mcPool.Push(RunThread, pcThread);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThreadPool::ThreadStateChanged(CThread* pcThread, EThreadState eState)
{
	if (eState == TS_Stopped)
	{
		mcThreads.Detach(pcThread);
		pcThread->Kill();
		mcThreads.FreeDetached(pcThread);
	}
}


