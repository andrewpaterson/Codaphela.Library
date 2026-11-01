#include <stdlib.h>
#include <string.h>
#include "BaseLib/Socket.h"
#include "BaseLib/NativeThread.h"
#include "ThreadLib/NetworkConnection.h"
#include "ThreadLib/SharedMemoryChannel.h"
#include "SharedMemoryQueueTheadElement.h"
#include "TestProcessChannel.h"


//The child process side of TestProcessChannel.  Nothing here depends on what
//carries the channel.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ChildReceive(CProcessChannel* pcChannel, uint32* puiType, void* pvData, size uiMaxSize, size* puiSize)
{
	//Times out so the child does not wait for ever if the parent fails.
	return pcChannel->Receive(puiType, pvData, uiMaxSize, puiSize, CHANNEL_TEST_TIMEOUT) == CR_Message;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessHello(CProcessChannel* pcChannel)
{
	char	szHello[] = "Hello World\n";

	return pcChannel->Send(CTM_Hello, szHello, strlen(szHello)) ? 0 : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessReverse(CProcessChannel* pcChannel)
{
	unsigned char	aucMemory[256];
	uint32			uiType;
	size			uiSize;
	size			i;

	if (!ChildReceive(pcChannel, &uiType, aucMemory, 256, &uiSize) || (uiType != CTM_Memory))
	{
		return 1;
	}

	for (i = 0; i < uiSize; i++)
	{
		aucMemory[i] = 255 - aucMemory[i];
	}

	return pcChannel->Send(CTM_Memory, aucMemory, uiSize) ? 0 : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessFill(CProcessChannel* pcChannel, char cFillChar, int iChunkSize)
{
	char		acChunk[CHANNEL_FILL_CHUNK_SIZE];
	uint32		uiType;
	uint32		uiPosition;

	if ((iChunkSize <= 0) || (iChunkSize > CHANNEL_FILL_CHUNK_SIZE))
	{
		return 1;
	}
	memset(acChunk, cFillChar, iChunkSize);

	for (;;)
	{
		if (!pcChannel->Send(CTM_FillRequest))
		{
			return 1;
		}

		if (!ChildReceive(pcChannel, &uiType, &uiPosition, sizeof(uint32), NULL))
		{
			return 1;
		}

		if (uiType == CTM_FillStop)
		{
			return 0;
		}
		else if (uiType != CTM_FillPosition)
		{
			return 1;
		}

		//The parent holds its lock until it has this chunk, as the shared memory test holds the mutex.
		if (!pcChannel->Send(CTM_FillChunk, acChunk, iChunkSize))
		{
			return 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessQueue(CProcessChannel* pcChannel, int iProcessNum)
{
	SSharedMemoryQueueElement	sElement;
	uint32						uiType;
	size						uiSize;

	if (!pcChannel->Send(CTM_QueueConsumer, &iProcessNum, sizeof(int)))
	{
		return 1;
	}

	for (;;)
	{
		if (!pcChannel->Send(CTM_QueuePop))
		{
			return 1;
		}

		if (!ChildReceive(pcChannel, &uiType, &sElement, sizeof(SSharedMemoryQueueElement), &uiSize))
		{
			return 1;
		}

		if (uiType == CTM_QueueElement)
		{
			if (!pcChannel->Send(CTM_QueueResult, &sElement, uiSize))
			{
				return 1;
			}
		}
		else if (uiType == CTM_QueueEmpty)
		{
			ThreadYield();
		}
		else if (uiType == CTM_QueueEnd)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SendLargeThenSmall(CProcessChannel* pcChannel)
{
	char	acLarge[CHANNEL_LARGE_SIZE];

	memset(acLarge, 'x', CHANNEL_LARGE_SIZE);
	if (!pcChannel->Send(CTM_Large, acLarge, CHANNEL_LARGE_SIZE))
	{
		return false;
	}
	return pcChannel->Send(CTM_Small, (void*)CHANNEL_SMALL_TEXT, strlen(CHANNEL_SMALL_TEXT) + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessTooLarge(CProcessChannel* pcChannel)
{
	STooLargeReport	sReport;
	char			acBuffer[CHANNEL_RECEIVE_SIZE];
	uint32			uiType;
	size			uiSize;

	//The parent receives these into a buffer that is too small.
	if (!SendLargeThenSmall(pcChannel))
	{
		return 1;
	}

	//The parent sends them too; report what arrived in a buffer that is too small.
	memset(&sReport, 0, sizeof(STooLargeReport));
	uiType = 0;
	uiSize = 0;
	sReport.iLargeResult = pcChannel->Receive(&uiType, acBuffer, CHANNEL_RECEIVE_SIZE, &uiSize, CHANNEL_TEST_TIMEOUT);
	sReport.uiLargeType = uiType;
	sReport.uiLargeSize = (uint32)uiSize;

	uiType = 0;
	uiSize = 0;
	sReport.iSmallResult = pcChannel->Receive(&uiType, sReport.szSmall, CHANNEL_RECEIVE_SIZE, &uiSize, CHANNEL_TEST_TIMEOUT);
	sReport.uiSmallType = uiType;
	sReport.uiSmallSize = (uint32)uiSize;

	return pcChannel->Send(CTM_TooLargeReport, &sReport, sizeof(STooLargeReport)) ? 0 : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestChannelProcessRun(CProcessChannel* pcChannel, int argc, char* argv[])
{
	char*	szTest;

	szTest = argv[0];
	if ((strcmp(szTest, "hello") == 0) && (argc == 3))
	{
		return TestChannelProcessHello(pcChannel);
	}
	else if ((strcmp(szTest, "reverse") == 0) && (argc == 3))
	{
		return TestChannelProcessReverse(pcChannel);
	}
	else if ((strcmp(szTest, "fill") == 0) && (argc == 5))
	{
		return TestChannelProcessFill(pcChannel, argv[3][0], atoi(argv[4]));
	}
	else if ((strcmp(szTest, "queue") == 0) && (argc == 4))
	{
		return TestChannelProcessQueue(pcChannel, atoi(argv[3]));
	}
	else if ((strcmp(szTest, "toolarge") == 0) && (argc == 3))
	{
		return TestChannelProcessTooLarge(pcChannel);
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	argv[0] is the test, argv[1] is "network" or "shared" and argv[2] is the
//	port or the channel name.  The rest depend on the test.
//
//////////////////////////////////////////////////////////////////////////
int TestProcessChannelMain(int argc, char* argv[])
{
	CNetworkConnection		cNetwork;
	CSharedMemoryChannel	cSharedMemory;
	CProcessChannel*		pcChannel;
	int						iResult;

	if (argc < 3)
	{
		return -1;
	}

	SocketsInit();
	cNetwork.Init();
	cSharedMemory.Init();

	pcChannel = NULL;
	if (strcmp(argv[1], "network") == 0)
	{
		if (cNetwork.Connect(NETWORK_LOOPBACK_ADDRESS, atoi(argv[2])))
		{
			pcChannel = &cNetwork;
		}
	}
	else if (strcmp(argv[1], "shared") == 0)
	{
		if (cSharedMemory.Connect(argv[2]))
		{
			pcChannel = &cSharedMemory;
		}
	}

	if (pcChannel)
	{
		iResult = TestChannelProcessRun(pcChannel, argc, argv);
		pcChannel->Close();
	}
	else
	{
		iResult = -1;
	}

	cSharedMemory.Kill();
	cNetwork.Kill();
	SocketsKill();
	return iResult;
}

