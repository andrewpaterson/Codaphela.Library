#include <thread>
#include "BaseLib/DebugOutput.h"
#include "Thread.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::Init(void)
{
	meState = TS_Waiting;
	miThreadId = -1;
	mpstdThread = NULL;
	mbDelete = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::Kill(void)
{
	miThreadId = -1;
	meState = TS_Killed;
	if (mbDelete)
	{
		if (mpstdThread->joinable())
		{
			mpstdThread->join();
		}
		delete mpstdThread;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RunThread(char* psz, CThread* pcThread)
{
	std::thread::id id = std::this_thread::get_id();
	std::thread::id* pid = &id;
	int	iid = *((int*)pid);

	pcThread->Start(iid, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::Start(void)
{
	char sz[] = { "Starting" };
	std::thread* pstdThread = new std::thread(RunThread, sz, this);
	
	SetThread(pstdThread);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::SetThread(std::thread* pstdThread)
{
	mpstdThread = pstdThread;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::Start(int iThreadId, BOOL bDelete)
{
	miThreadId = iThreadId;
	mbDelete = bDelete;
	meState = TS_Running;
	Run();
	meState = TS_Stopped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CThread::IsDone(void)
{
	return meState == TS_Stopped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CThread::IsRunning(void)
{
	return meState == TS_Running;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CThread::GetThreadId(void)
{
	return miThreadId;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CThread::TryStop(void)
{
	if (meState == TS_Running)
	{
		meState = TS_Stopping;
	}
}


