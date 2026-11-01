#include "BaseLib/NativeThread.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/Logger.h"
#include "ThreadLib/SharedMemoryQueue.h"
#include "ThreadLib/InterProcessDone.h"
#include "TestSharedMemoryQueue.h"
#include "SharedMemoryQueueTheadElement.h"
#include "BaseLib/WindowsHeaders.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestSharedMemoryQueue(char* szSharedQueueName, char* szSharedResultName, char* szProducersDoneName, int iProcessNum)
{
	CSharedMemory				cResult;
	SProcessSharedQueueResult*	psResult;
	bool						bResult;
	SSharedMemoryQueueElement	sConsumerData;
	size							uiSize;
	CSharedMemoryQueue			cQueue;
	CInterProcessDone			cProcessesDone;
	int							iTaken;

	cResult.Init(szSharedResultName);
	psResult = (SProcessSharedQueueResult*)cResult.Open();

	cProcessesDone.Init(szProducersDoneName);
	cProcessesDone.Open();

	cQueue.Init(szSharedQueueName);

	iTaken = 0;
	do
	{
		bResult = cQueue.Pop(&sConsumerData, &uiSize, sizeof(SSharedMemoryQueueElement));
		if (bResult)
		{
			strcpy(psResult->aszData[sConsumerData.sHeader.miIndex], sConsumerData.sz);
			iTaken++;
		}
		else
		{
			ThreadYield();
		}
	} while ((!cProcessesDone.IsDone(1)) || bResult);


	cQueue.Kill();

	psResult->aiTaken[iProcessNum] = iTaken;
	cResult.Close();
	cResult.Kill();

	cProcessesDone.Stop();
	cProcessesDone.Kill();

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestSharedQueueProcessMain(int argc, char* argv[])
{
	if (argc == 4)
	{
		return TestSharedMemoryQueue(argv[0], argv[1], argv[2], atoi(argv[3]));
	}
	else
	{
		return -1;
	}
}

