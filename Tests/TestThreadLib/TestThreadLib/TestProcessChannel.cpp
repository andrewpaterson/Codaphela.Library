#include <stdlib.h>
#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChar.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/Socket.h"
#include "BaseLib/ListenSocket.h"
#include "ThreadLib/AtomicInt.h"
#include "BaseLib/Mutex.h"
#include "ThreadLib/NetworkConnection.h"
#include "ThreadLib/SharedMemoryChannel.h"
#include "ThreadLib/ProcessFork.h"
#include "ThreadLib/WorkerThreadPool.h"
#include "TestLib/Assert.h"
#include "SharedMemoryQueueTheadElement.h"
#include "TestSharedMemoryQueue.h"
#include "TestProcessChannel.h"


void AssertFilledMemory(char* pcMemory, int iChunkSize);


//The inter process tests of TestProcessFork, TestSharedMemory and
//TestSharedMemoryQueue written once against CProcessChannel and run over the
//loopback network and over shared memory.  The parent process owns the memory
//and serves each forked process on its own pool thread, so a CMutex in the
//parent replaces the CInterProcessMutex.


//Channels to forked processes.  Only Init and Fork know what carries them.
class CTestChannels
{
public:
	ETestTransport			meTransport;
	CListenSocket			mcListen;
	char					mszPort[16];
	CNetworkConnection		macNetwork[4];
	CSharedMemoryChannel	macSharedMemory[4];
	char					maszNames[4][32];
	bool					mabConnected[4];

	bool				Init(ETestTransport eTransport);
	void				Kill(void);

	bool				Fork(int i, char* szTest, char* szParameter1 = NULL, char* szParameter2 = NULL);
	CProcessChannel*	Get(int i);
};


int giTestChannelNames = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTestChannels::Init(ETestTransport eTransport)
{
	int		i;
	int		iPort;

	meTransport = eTransport;
	for (i = 0; i < 4; i++)
	{
		macNetwork[i].Init();
		macSharedMemory[i].Init();
		maszNames[i][0] = '\0';
		mabConnected[i] = false;
	}

	if (meTransport == TT_Network)
	{
		//Listening before the processes are forked so they can connect at once.
		if (!mcListen.Init(NETWORK_LOOPBACK_ADDRESS, 0))
		{
			return false;
		}
		iPort = mcListen.GetPort();
		IntToString(mszPort, 16, iPort);
		return iPort > 0;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestChannels::Kill(void)
{
	int		i;

	for (i = 0; i < 4; i++)
	{
		macNetwork[i].Kill();
		macSharedMemory[i].Kill();
	}

	if (meTransport == TT_Network)
	{
		mcListen.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTestChannels::Fork(int i, char* szTest, char* szParameter1, char* szParameter2)
{
	char*	szTransport;
	char*	szAddress;
	CChars	sz;

	if (meTransport == TT_Network)
	{
		szTransport = "network";
		szAddress = mszPort;
	}
	else
	{
		//A new name each time so a process from an earlier test still closing it does not matter.
		sz.Init("TestChannel")->Append(giTestChannelNames);
		giTestChannelNames++;
		StrCpySafe(maszNames[i], sz.Text(), 32);
		sz.Kill();
		if (!macSharedMemory[i].Create(maszNames[i]))
		{
			return false;
		}
		szTransport = "shared";
		szAddress = maszNames[i];
	}

	if (szParameter2)
	{
		ForkProcess("--test-channel", szTest, szTransport, szAddress, szParameter1, szParameter2, false);
	}
	else if (szParameter1)
	{
		ForkProcess("--test-channel", szTest, szTransport, szAddress, szParameter1, false);
	}
	else
	{
		ForkProcess("--test-channel", szTest, szTransport, szAddress, false);
	}

	if (meTransport == TT_Network)
	{
		mabConnected[i] = macNetwork[i].Accept(&mcListen, CHANNEL_TEST_TIMEOUT);
	}
	else
	{
		//The forked process connects by name; its first message shows it did.
		mabConnected[i] = true;
	}
	return mabConnected[i];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CProcessChannel* CTestChannels::Get(int i)
{
	if (meTransport == TT_Network)
	{
		return &macNetwork[i];
	}
	else
	{
		return &macSharedMemory[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ParentReceive(CProcessChannel* pcChannel, uint32* puiType, void* pvData, size uiMaxSize, size* puiSize)
{
	return pcChannel->Receive(puiType, pvData, uiMaxSize, puiSize, CHANNEL_TEST_TIMEOUT) == CR_Message;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelProcessFork(ETestTransport eTransport)
{
	CTestChannels	cChannels;
	char			szHello[64];
	uint32			uiType;
	size			uiSize;
	bool			bResult;

	AssertTrue(cChannels.Init(eTransport));

	bResult = cChannels.Fork(0, "hello");
	AssertTrue(bResult);
	if (bResult)
	{
		AssertTrue(ParentReceive(cChannels.Get(0), &uiType, szHello, 63, &uiSize));
		AssertInt(CTM_Hello, uiType);
		szHello[uiSize] = '\0';
		AssertString("Hello World\n", szHello);

		//Every message has been received so the next Receive finds it closed.
		AssertInt(CR_Closed, cChannels.Get(0)->Receive(&uiType, szHello, 63, &uiSize, CHANNEL_TEST_TIMEOUT));
	}

	cChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelTwoProcesses(ETestTransport eTransport)
{
	CTestChannels		cChannels;
	CProcessChannel*	pcChannel;
	unsigned char		aucMemory[256];
	uint32				uiType;
	size				uiSize;
	int					i;
	bool				bResult;

	AssertTrue(cChannels.Init(eTransport));

	bResult = cChannels.Fork(0, "reverse");
	AssertTrue(bResult);
	if (bResult)
	{
		pcChannel = cChannels.Get(0);
		for (i = 0; i < 256; i++)
		{
			aucMemory[i] = (unsigned char)i;
		}
		AssertTrue(pcChannel->Send(CTM_Memory, aucMemory, 256));

		memset(aucMemory, 0, 256);
		AssertTrue(ParentReceive(pcChannel, &uiType, aucMemory, 256, &uiSize));
		AssertInt(CTM_Memory, uiType);
		AssertInt(256, (int)uiSize);
		for (i = 0; i < 256; i++)
		{
			AssertChar(255 - (unsigned char)i, aucMemory[i]);
		}
	}

	cChannels.Kill();
}


bool SendLargeThenSmall(CProcessChannel* pcChannel);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelTooLarge(ETestTransport eTransport)
{
	CTestChannels		cChannels;
	CProcessChannel*	pcChannel;
	char				acBuffer[CHANNEL_RECEIVE_SIZE];
	STooLargeReport		sReport;
	uint32				uiType;
	size				uiSize;
	bool				bResult;

	AssertTrue(cChannels.Init(eTransport));

	bResult = cChannels.Fork(0, "toolarge");
	AssertTrue(bResult);
	if (bResult)
	{
		pcChannel = cChannels.Get(0);

		//The message is dropped but its type and size are returned.
		uiType = 0;
		uiSize = 0;
		AssertInt(CR_TooLarge, pcChannel->Receive(&uiType, acBuffer, CHANNEL_RECEIVE_SIZE, &uiSize, CHANNEL_TEST_TIMEOUT));
		AssertInt(CTM_Large, uiType);
		AssertInt(CHANNEL_LARGE_SIZE, (int)uiSize);

		//The channel still works and the next message is complete.
		AssertInt(CR_Message, pcChannel->Receive(&uiType, acBuffer, CHANNEL_RECEIVE_SIZE, &uiSize, CHANNEL_TEST_TIMEOUT));
		AssertInt(CTM_Small, uiType);
		AssertInt((int)strlen(CHANNEL_SMALL_TEXT) + 1, (int)uiSize);
		AssertString(CHANNEL_SMALL_TEXT, acBuffer);

		//The same the other way, received by the child.
		AssertTrue(SendLargeThenSmall(pcChannel));
		AssertTrue(ParentReceive(pcChannel, &uiType, &sReport, sizeof(STooLargeReport), &uiSize));
		AssertInt(CTM_TooLargeReport, uiType);
		AssertInt((int)sizeof(STooLargeReport), (int)uiSize);
		AssertInt(CR_TooLarge, sReport.iLargeResult);
		AssertInt(CTM_Large, (int)sReport.uiLargeType);
		AssertInt(CHANNEL_LARGE_SIZE, (int)sReport.uiLargeSize);
		AssertInt(CR_Message, sReport.iSmallResult);
		AssertInt(CTM_Small, (int)sReport.uiSmallType);
		AssertInt((int)strlen(CHANNEL_SMALL_TEXT) + 1, (int)sReport.uiSmallSize);
		AssertString(CHANNEL_SMALL_TEXT, sReport.szSmall);
	}

	cChannels.Kill();
}


struct SChannelFillServer
{
	CMutex		cMutex;
	CArrayChar	acMemory;
	uint32		uiPosition;
	uint32		uiStop;
	int			iChunkSize;
};


struct SChannelFillClient
{
	CProcessChannel*		pcChannel;
	SChannelFillServer*		psServer;
	bool					bResult;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ServeChannelFillChunk(SChannelFillClient* psClient, bool* pbStop)
{
	SChannelFillServer*	psServer;
	CProcessChannel*	pcChannel;
	char				acChunk[CHANNEL_FILL_CHUNK_SIZE];
	uint32				uiType;
	size				uiSize;

	psServer = psClient->psServer;
	pcChannel = psClient->pcChannel;
	*pbStop = false;

	if (!ParentReceive(pcChannel, &uiType, NULL, 0, NULL) || (uiType != CTM_FillRequest))
	{
		return false;
	}

	//Held while the child writes its chunk, as the shared memory test holds its mutex.
	psServer->cMutex.Lock();
	if (psServer->uiPosition == psServer->uiStop)
	{
		psServer->cMutex.Unlock();
		*pbStop = true;
		return pcChannel->Send(CTM_FillStop);
	}

	if (!pcChannel->Send(CTM_FillPosition, &psServer->uiPosition, sizeof(uint32)) ||
		!ParentReceive(pcChannel, &uiType, acChunk, CHANNEL_FILL_CHUNK_SIZE, &uiSize) ||
		(uiType != CTM_FillChunk) ||
		(uiSize != (size)psServer->iChunkSize))
	{
		psServer->cMutex.Unlock();
		return false;
	}

	psServer->acMemory.Resize(psServer->uiPosition + uiSize);
	memcpy(psServer->acMemory.Get(psServer->uiPosition), acChunk, uiSize);
	psServer->uiPosition += (uint32)uiSize;
	psServer->cMutex.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ServeChannelFill(int iThreadId, void* pvClient)
{
	SChannelFillClient*	psClient;
	bool				bStop;

	psClient = (SChannelFillClient*)pvClient;
	for (;;)
	{
		if (!ServeChannelFillChunk(psClient, &bStop))
		{
			psClient->bResult = false;
			return;
		}

		if (bStop)
		{
			psClient->bResult = true;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelResizeMultiProcess(ETestTransport eTransport)
{
	CTestChannels		cChannels;
	SChannelFillServer	sServer;
	SChannelFillClient	asClients[4];
	CWorkerThreadPool	cPool;
	char				szChunkSize[16];
	char				aszFillChar[4][2] = { "A", "B", "C", "D" };
	int					i;
	int					iConnected;

	sServer.cMutex.Init();
	sServer.acMemory.Init();
	sServer.uiPosition = 0;
	sServer.uiStop = CHANNEL_FILL_SIZE;
	sServer.iChunkSize = CHANNEL_FILL_CHUNK_SIZE;
	IntToString(szChunkSize, 16, sServer.iChunkSize);

	AssertTrue(cChannels.Init(eTransport));
	cPool.Init(4);
	iConnected = 0;
	for (i = 0; i < 4; i++)
	{
		asClients[i].pcChannel = cChannels.Get(i);
		asClients[i].psServer = &sServer;
		asClients[i].bResult = false;
		if (cChannels.Fork(i, "fill", aszFillChar[i], szChunkSize))
		{
			cPool.Push(ServeChannelFill, &asClients[i]);
			iConnected++;
		}
	}
	cPool.Stop(true);
	cPool.Kill();

	AssertInt(4, iConnected);
	for (i = 0; i < 4; i++)
	{
		AssertTrue(asClients[i].bResult);
	}

	AssertInt(CHANNEL_FILL_SIZE, sServer.uiPosition);
	AssertInt(CHANNEL_FILL_SIZE, (int)sServer.acMemory.NumElements());
	if (sServer.acMemory.NumElements() == CHANNEL_FILL_SIZE)
	{
		AssertFilledMemory(sServer.acMemory.GetData(), sServer.iChunkSize);
	}

	cChannels.Kill();
	sServer.acMemory.Kill();
	sServer.cMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelResizeMultiProcessRepeat(ETestTransport eTransport, int iIterations)
{
	int i;

	for (i = 0; i < iIterations; i++)
	{
		TestChannelResizeMultiProcess(eTransport);
	}
}


struct SChannelQueueServer
{
	SSharedMemoryQueueElement*	asElements;
	CAtomicInt					iProduced;
	CAtomicInt					iNextToTake;
	CAtomicInt					iProducerDone;
	SProcessSharedQueueResult*	psResult;
};


struct SChannelQueueClient
{
	CProcessChannel*		pcChannel;
	SChannelQueueServer*	psServer;
	bool					bResult;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSharedMemoryQueueElement* TakeChannelQueueElement(SChannelQueueServer* psServer, bool* pbEnd)
{
	int		iNext;
	bool	bProducerDone;

	for (;;)
	{
		//Read done before produced so nothing produced in between is missed.
		bProducerDone = psServer->iProducerDone.Get() != 0;
		iNext = psServer->iNextToTake.Get();
		if (iNext >= psServer->iProduced.Get())
		{
			*pbEnd = bProducerDone;
			return NULL;
		}

		if (psServer->iNextToTake.CompareExchange(iNext, iNext + 1))
		{
			*pbEnd = false;
			return &psServer->asElements[iNext];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ServeChannelQueue(int iThreadId, void* pvClient)
{
	SChannelQueueClient*		psClient;
	SChannelQueueServer*		psServer;
	CProcessChannel*			pcChannel;
	SSharedMemoryQueueElement	sElement;
	SSharedMemoryQueueElement*	psElement;
	uint32						uiType;
	size						uiSize;
	int							iProcessNum;
	bool						bEnd;
	bool						bResult;

	psClient = (SChannelQueueClient*)pvClient;
	psServer = psClient->psServer;
	pcChannel = psClient->pcChannel;
	iProcessNum = -1;

	for (;;)
	{
		if (!ParentReceive(pcChannel, &uiType, &sElement, sizeof(SSharedMemoryQueueElement), &uiSize))
		{
			psClient->bResult = false;
			return;
		}

		if ((uiType == CTM_QueueConsumer) && (uiSize == sizeof(int)))
		{
			memcpy(&iProcessNum, &sElement, sizeof(int));
			bResult = (iProcessNum >= 0) && (iProcessNum < 4);
		}
		else if ((uiType == CTM_QueuePop) && (iProcessNum != -1))
		{
			psElement = TakeChannelQueueElement(psServer, &bEnd);
			if (psElement)
			{
				bResult = pcChannel->Send(CTM_QueueElement, psElement, psElement->sHeader.muiSize);
			}
			else if (bEnd)
			{
				psClient->bResult = pcChannel->Send(CTM_QueueEnd);
				return;
			}
			else
			{
				bResult = pcChannel->Send(CTM_QueueEmpty);
			}
		}
		else if ((uiType == CTM_QueueResult) && (iProcessNum != -1) && (sElement.sHeader.miIndex >= 0) && (sElement.sHeader.miIndex < PROCESS_SHARED_QUEUE_PRODUCE_SIZE))
		{
			//Each index is taken once so only this thread writes it.
			strcpy(psServer->psResult->aszData[sElement.sHeader.miIndex], sElement.sz);
			psServer->psResult->aiTaken[iProcessNum]++;
			bResult = true;
		}
		else
		{
			bResult = false;
		}

		if (!bResult)
		{
			psClient->bResult = false;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ForkChannelQueueConsumer(CTestChannels* pcChannels, int iProcess, SChannelQueueClient* psClient, SChannelQueueServer* psServer, CWorkerThreadPool* pcPool)
{
	char	szProcess[16];

	psClient->pcChannel = pcChannels->Get(iProcess);
	psClient->psServer = psServer;
	psClient->bResult = false;
	if (pcChannels->Fork(iProcess, "queue", IntToString(szProcess, 16, iProcess)))
	{
		pcPool->Push(ServeChannelQueue, psClient);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//	bForkEarly forks the consumers before producing, as in
//	TestSharedMemoryQueueMultiProcessForkEarly, otherwise one is forked after
//	every 1000 elements are produced, as in ...ForkDuring.
//
//////////////////////////////////////////////////////////////////////////
void TestChannelQueueMultiProcess(ETestTransport eTransport, bool bForkEarly)
{
	CTestChannels			cChannels;
	SChannelQueueServer		sServer;
	SChannelQueueClient		asClients[4];
	CWorkerThreadPool		cPool;
	int						iProcess;
	int						iConnected;
	int						iProduce;
	SQueueTestSequence		sSequence;
	int						i;

	sServer.asElements = (SSharedMemoryQueueElement*)malloc(PROCESS_SHARED_QUEUE_PRODUCE_SIZE * sizeof(SSharedMemoryQueueElement));
	sServer.psResult = (SProcessSharedQueueResult*)malloc(sizeof(SProcessSharedQueueResult));
	memset(sServer.psResult, 0, sizeof(SProcessSharedQueueResult));
	sServer.iProduced.Set(0);
	sServer.iNextToTake.Set(0);
	sServer.iProducerDone.Set(0);

	AssertTrue(cChannels.Init(eTransport));
	cPool.Init(4);

	iConnected = 0;
	iProcess = 0;
	if (bForkEarly)
	{
		for (iProcess = 0; iProcess < 4; iProcess++)
		{
			if (ForkChannelQueueConsumer(&cChannels, iProcess, &asClients[iProcess], &sServer, &cPool))
			{
				iConnected++;
			}
		}
	}

	sSequence.Init();
	for (iProduce = 0; iProduce < PROCESS_SHARED_QUEUE_PRODUCE_SIZE; iProduce++)
	{
		FillQueueElement(&sServer.asElements[iProduce], iProduce, &sSequence);
		sServer.iProduced.Increment();

		if (!bForkEarly && (iProduce % 1000 == 999) && (iProcess < 4))
		{
			if (ForkChannelQueueConsumer(&cChannels, iProcess, &asClients[iProcess], &sServer, &cPool))
			{
				iConnected++;
			}
			iProcess++;
		}
	}
	sServer.iProducerDone.Set(1);

	cPool.Stop(true);
	cPool.Kill();

	AssertInt(4, iConnected);
	for (i = 0; i < 4; i++)
	{
		AssertTrue(asClients[i].bResult);
	}

	AssertQueueResult(sServer.psResult->aiTaken, sServer.psResult->aszData, PROCESS_SHARED_QUEUE_PRODUCE_SIZE);


	cChannels.Kill();
	free(sServer.psResult);
	free(sServer.asElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessChannel(ETestTransport eTransport)
{
	TestChannelProcessFork(eTransport);
	TestChannelTwoProcesses(eTransport);
	TestChannelTooLarge(eTransport);
	TestChannelResizeMultiProcessRepeat(eTransport, 100);
	TestChannelQueueMultiProcess(eTransport, false);
	TestChannelQueueMultiProcess(eTransport, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessChannel(void)
{
	BeginTests();
	SocketsInit();

	TestProcessChannel(TT_Network);
	TestProcessChannel(TT_SharedMemory);

	SocketsKill();
	TestStatistics();
}

