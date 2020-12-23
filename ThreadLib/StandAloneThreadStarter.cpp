#include <thread>
#include "StandAloneThreadStarter.h"
#include "Thread.h"


CStandAloneThreadStarter gcStandAloneThreadStarter;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RunThread(CThread* pcThread)
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
void CStandAloneThreadStarter::StartThread(CThread* pcThread)
{
	std::thread* pstdThread = new std::thread(RunThread, pcThread);

	pcThread->SetThread(pstdThread);
}

