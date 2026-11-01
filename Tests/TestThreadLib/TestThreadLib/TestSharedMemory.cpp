#include "BaseLib/NativeThread.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StdRandom.h"
#include "ThreadLib/ResizableSharedMemory.h"
#include "ThreadLib/InterProcessMutex.h"
#include "ThreadLib/ProcessFork.h"
#include "ThreadLib/InterProcessHold.h"
#include "ThreadLib/InterProcessDone.h"
#include "ThreadLib/Thread.h"
#include "ThreadLib/ThreadsDone.h"
#include "TestLib/Assert.h"
#include "SharedMemoryThread.h"
#include "TestTimeout.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertMappedFile(char* szName, int iExpectedMappedCount)
{
	CSharedMemoryFile			cMappedFile;
	SSharedMemoryResult			sResult;
	SSharedMemoryDescriptor*	psDescriptor;

	cMappedFile.Init(szName);
	sResult = cMappedFile.Open();

	if (iExpectedMappedCount > 0)
	{
		if (sResult.IsFailed())
		{
			AssertTrue(sResult.IsSuccess());
		}
		else
		{
			psDescriptor = cMappedFile.Map(0);
			AssertInt(psDescriptor->iMapCount, iExpectedMappedCount);
			cMappedFile.Unmap(psDescriptor);
			cMappedFile.Close();
		}
	}
	else
	{
		if (sResult.IsSuccess())
		{
			cMappedFile.Close();
		}
		AssertTrue(sResult.IsFailed());
	}
	cMappedFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryOneProcess(void)
{
	CResizableSharedMemory	cSharedOwner;
	CResizableSharedMemory	cSharedClient;
	bool			bResult;
	char*			pcOwner;
	char*			pcClient;
	int				i;

	cSharedOwner.Init("Local\\TestSharedMemoryOneProcess");
	bResult = cSharedOwner.Create(256);
	AssertTrue(bResult);

	pcOwner = (char*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(pcOwner);

	for (i = 0; i < 256; i++)
	{
		pcOwner[i] = (char)i;
	}

	cSharedClient.Init("Local\\TestSharedMemoryOneProcess");
	bResult = cSharedClient.Connect();
	AssertTrue(bResult);

	pcClient = (char*)cSharedClient.Touch().pvMemory;
	AssertNotNull(pcClient);

	for (i = 0; i < 256; i++)
	{
		AssertChar((char)i, pcClient[i]);
	}
	AssertMappedFile("Local\\TestSharedMemoryOneProcess:0", 2);

	cSharedOwner.Close();
	cSharedOwner.Kill();
	AssertMappedFile("Local\\TestSharedMemoryOneProcess:0", 1);

	cSharedClient.Close();
	cSharedClient.Kill();
	AssertMappedFile("Local\\TestSharedMemoryOneProcess:0", 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeOneProcess(void)
{
	CResizableSharedMemory	cSharedOwner;
	CResizableSharedMemory	cSharedClient;
	CResizableSharedMemory	cSharedNewOwner;
	bool			bResult;
	char*			pcOwner;
	char*			pcClient;
	int				i;
	int				iStillMapped;
	SLogConfig		sLogConfig;

	cSharedOwner.Init("Local\\TestSharedMemoryResizeOneProcess");
	bResult = cSharedOwner.Create(3940);
	AssertTrue(bResult);

	pcOwner = (char*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(pcOwner);

	for (i = 0; i < 3940; i++)
	{
		pcOwner[i] = (char)i;
	}

	cSharedClient.Init("Local\\TestSharedMemoryResizeOneProcess");
	pcClient = (char*)cSharedClient.Touch().pvMemory;
	AssertNotNull(pcClient);

	AssertNotNull(pcClient);
	for (i = 0; i < 3940; i++)
	{
		AssertChar((char)i, pcClient[i]);
	}

	cSharedOwner.Close();

	cSharedNewOwner.Init("Local\\TestSharedMemoryResizeOneProcess");
	gcLogger.GetConfig(&sLogConfig);
	gcLogger.SetBreakOnError(false);
	bResult = cSharedNewOwner.Create(3940 + 16);
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(bResult);
	cSharedNewOwner.Kill();

	pcClient = (char*)cSharedClient.Resize(3940 + 16).pvMemory;
	AssertNotNull(pcClient);
	for (i = 0; i < 3940; i++)
	{
		AssertChar((char)i, pcClient[i]);
	}

	pcOwner = (char*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(pcOwner);
	for (i = 0; i < 3940; i++)
	{
		AssertChar((char)i, pcOwner[i]);
	}

	iStillMapped = cSharedOwner.Close();
	AssertInt(1, iStillMapped);
	iStillMapped = cSharedClient.Close();
	AssertInt(0, iStillMapped);

	cSharedNewOwner.Init("Local\\TestSharedMemoryResizeOneProcess");
	bResult = cSharedNewOwner.Connect();
	AssertTrue(bResult);
	cSharedNewOwner.Close();
	cSharedNewOwner.Kill();

	cSharedOwner.Kill();
	cSharedClient.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryTwoProcesses(void)
{
	CResizableSharedMemory	cSharedOwner;
	bool			bResult;
	unsigned char*	pcOwner;
	int				i, j;
	char			szSharedMemoryName[] = {"Local\\TestSharedMemoryTwoProcesses"};
	bool			bAllMatch;

	cSharedOwner.Init(szSharedMemoryName);
	bResult = cSharedOwner.Create(256);
	AssertTrue(bResult);

	pcOwner = (unsigned char*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(pcOwner);

	for (i = 0; i < 256; i++)
	{
		pcOwner[i] = (char)i;
	}

	ForkProcess("--test-shared-memory", szSharedMemoryName, "256", false);

	for (j = 0; j < 50; j++)
	{
		bAllMatch = true;
		for (i = 0; i < 256; i++)
		{
			if (255 - (unsigned char)i != pcOwner[i])
			{
				bAllMatch = false;
				break;
			}
		}

		if (bAllMatch)
		{
			Pass();
			break;
		}
		ThreadSleep(16);
	}
	AssertTrue(bAllMatch);

	for (i = 0; i < 256; i++)
	{
		AssertChar(255 - (unsigned char)i, pcOwner[i]);
	}

	cSharedOwner.Close();
	cSharedOwner.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFilledMemory(char* pcMemory, int iChunkSize)
{
	int		iA;
	int		iB;
	int		iC;
	int		iD;
	bool	bOneEqualsChunkSize;
	bool	bSumEqualsChunkSize;
	int		i;
	CChars	sz;

	bOneEqualsChunkSize = true;
	bSumEqualsChunkSize = true;

	for (i = 0; i < 16000 / iChunkSize; i++)
	{
		sz.Init();
		sz.Append(pcMemory, iChunkSize);
		sz.AppendNewLine();

		iA = (int)sz.Count('A');
		iB = (int)sz.Count('B');
		iC = (int)sz.Count('C');
		iD = (int)sz.Count('D');
		sz.Kill();

		bOneEqualsChunkSize = (iA == iChunkSize || iB == iChunkSize || iC == iChunkSize || iD == iChunkSize);
		bSumEqualsChunkSize = (iA + iB + iC + iD == iChunkSize);

		if (!bOneEqualsChunkSize || !bSumEqualsChunkSize)
		{
			break;
		}

		pcMemory = (char*)RemapSinglePointer(pcMemory, iChunkSize);
	}

	AssertTrue(bOneEqualsChunkSize);
	AssertTrue(bSumEqualsChunkSize);
	Pass();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeOneProcessFill(void)
{
	CResizableSharedMemory	cSharedOwner;
	CResizableSharedMemory	cSharedClient[4];
	CResizableSharedMemory*	pcSharedClient;
	bool					bResult;
	char*					pcMemory;
	int						i;
	int						iChunkSize;
	unsigned int*			puiPosition;
	char					szMemoryName[] = { "Local\\TestSharedMemoryResizeOneProcessFill" };
	CRandom					cRandom;
	unsigned int			uiStop;
	char					cFillChar;
	void*					pvDest;

	iChunkSize = 128;
	uiStop = 16000 + sizeof(int);

	cSharedOwner.Init(szMemoryName);
	bResult = cSharedOwner.Create(sizeof(int) + iChunkSize);
	AssertTrue(bResult);

	pcMemory = (char*)cSharedOwner.Touch().pvMemory;
	memset(pcMemory, 0, sizeof(int) + iChunkSize);
	puiPosition = (unsigned int*)pcMemory;
	*puiPosition = 4;

	for (i = 0; i < 4; i++)
	{
		cSharedClient[i].Init(szMemoryName);
	}

	cSharedOwner.Close();

	cRandom.Init(77);

	for (;;)
	{
		i = cRandom.Next(0, 3);
		pcSharedClient = &cSharedClient[i];
		cFillChar = 'A' + (char)i;

		puiPosition = (unsigned int*)pcSharedClient->Touch().pvMemory;
		if (puiPosition == NULL)
		{
			pcSharedClient->Close();
			pcSharedClient->Kill();
			AssertTrue(false);
			break;
		}

		if ((*puiPosition) == uiStop)
		{
			pcSharedClient->Close();
			pcSharedClient->Kill();
			break;
		}

		pvDest = RemapSinglePointer(puiPosition, (*puiPosition));
		memset(pvDest, cFillChar, iChunkSize);

		(*puiPosition) = (*puiPosition) + iChunkSize;

		pcSharedClient->Resize((*puiPosition) + iChunkSize);
	}

	cSharedOwner.Connect();
	for (i = 0; i < 4; i++)
	{
		cSharedClient[i].Close();
		cSharedClient[i].Kill();
	}

	pcMemory = (char*)RemapSinglePointer(cSharedOwner.Touch().pvMemory, sizeof(int));
	AssertFilledMemory(pcMemory, iChunkSize);

	cSharedOwner.Close();
	cSharedOwner.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeOneProcessResizeAndRelease(void)
{
	CResizableSharedMemory	cSharedOwner;
	CResizableSharedMemory	cSharedClient[4];
	bool					bResult;
	int						i;
	char					szMemoryName[] = { "Local\\TestSharedMemoryResizeOneProcessResizeAndRelease" };

	cSharedOwner.Init(szMemoryName);
	bResult = cSharedOwner.Create(3000);
	AssertTrue(bResult);

	cSharedOwner.Touch().pvMemory;

	for (i = 0; i < 4; i++)
	{
		cSharedClient[i].Init(szMemoryName);
		cSharedClient[i].Touch().pvMemory;
	}

	cSharedClient[0].Resize(6700);
	cSharedClient[1].Touch().pvMemory;
	cSharedClient[2].Touch().pvMemory;
	cSharedClient[3].Touch().pvMemory;

	for (i = 0; i < 4; i++)
	{
		cSharedClient[i].Close();
	}

	cSharedOwner.Touch().pvMemory;
	cSharedOwner.Close();

	for (i = 0; i < 4; i++)
	{
		cSharedClient[i].Kill();
	}
	cSharedOwner.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeMultiThread(void)
{
	CResizableSharedMemory	cSharedOwner;
	CInterProcessMutex		cMutex;
	bool					bResult;
	char					szSharedMemoryName[] = { "Local\\TestSharedMemoryResizeMultiThread" };
	char					szMutexName[] = { "Local\\TestMutexResizeMultiThread" };
	unsigned int*			puiPosition;
	unsigned int			uiPosition;
	CSharedMemoryThread		cThreadA;
	CSharedMemoryThread		cThreadB;
	CSharedMemoryThread		cThreadC;
	CSharedMemoryThread		cThreadD;
	int						iChunkSize;
	CInterProcessHold		cHold;
	CInterProcessDone		cDone;
	char*					pcMemory;

	cMutex.Init(szMutexName);
	bResult = cMutex.Create();
	AssertTrue(bResult);
	iChunkSize = 128;

	cSharedOwner.Init(szSharedMemoryName, "Owner");
	bResult = cSharedOwner.Create(sizeof(int) + iChunkSize);
	AssertTrue(bResult);

	puiPosition = (unsigned int*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(puiPosition);

	memset(puiPosition, 0, sizeof(int) + iChunkSize);
	*puiPosition = sizeof(int);

	cThreadA.Init(szSharedMemoryName, szMutexName, "A", iChunkSize);
	cThreadB.Init(szSharedMemoryName, szMutexName, "B", iChunkSize);
	cThreadC.Init(szSharedMemoryName, szMutexName, "C", iChunkSize);
	cThreadD.Init(szSharedMemoryName, szMutexName, "D", iChunkSize);

	cHold.Init(szSharedMemoryName, ":H");
	cHold.Start();

	cDone.Init(szSharedMemoryName, ":D");
	cDone.Start();

	cThreadA.Start();
	cThreadB.Start();
	cThreadC.Start();
	cThreadD.Start();

	for (;;)
	{
		bResult = cMutex.Lock();

		puiPosition = (unsigned int*)cSharedOwner.Touch().pvMemory;
		if (!puiPosition)
		{
			AssertNotNull(puiPosition);
		}
		uiPosition = *puiPosition;

		bResult = cMutex.Unlock();

		if (uiPosition == 16000 + sizeof(int))
		{
			break;
		}

		ThreadYield();
	}

	cHold.Stop();
	AssertTrue(cDone.Wait(4, TEST_TIMEOUT));
	cHold.Kill();
	cDone.Kill();

	cThreadA.Kill();
	cThreadB.Kill();
	cThreadC.Kill();
	cThreadD.Kill();

	pcMemory = (char*)RemapSinglePointer(cSharedOwner.Touch().pvMemory, sizeof(int));
	AssertFilledMemory(pcMemory, iChunkSize);

	cSharedOwner.Close();
	cSharedOwner.Kill();

	cMutex.Close();
	cMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeMultiThreadRepeat(int iIterations)
{
	int i;

	for (i = 0; i < iIterations; i++)
	{
		TestSharedMemoryResizeMultiThread();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeMultiProcess(void)
{
	CResizableSharedMemory		cSharedOwner;
	CInterProcessMutex			cMutex;
	bool						bResult;
	char						szSharedMemoryName[] = { "Local\\TestSharedMemoryResizeMultiProcess" };
	char						szMutexName[] = { "Local\\TestMutexResizeMultiProcess" };
	unsigned int*				puiPosition;
	unsigned int				uiPosition;
	int							iChunkSize;
	char						szChunkSize[16];
	CInterProcessHold			cHold;
	CInterProcessDone			cDone;
	char*						pcMemory;

	cMutex.Init(szMutexName);
	bResult = cMutex.Create();
	AssertTrue(bResult);
	iChunkSize = 16;
	IntToString(szChunkSize, 16, iChunkSize);

	cSharedOwner.Init(szSharedMemoryName, "Owner");
	bResult = cSharedOwner.Create(sizeof(int) + iChunkSize);
	AssertTrue(bResult);

	puiPosition = (unsigned int*)cSharedOwner.Touch().pvMemory;
	AssertNotNull(puiPosition);

	memset(puiPosition, 0, sizeof(int) + iChunkSize);
	*puiPosition = sizeof(int);

	cHold.Init(szSharedMemoryName, ":H");
	cHold.Start();

	cDone.Init(szSharedMemoryName, ":D");
	cDone.Start();

	ForkProcess("--test-shared-memory", szSharedMemoryName, szMutexName, "A", szChunkSize, false);
	ForkProcess("--test-shared-memory", szSharedMemoryName, szMutexName, "B", szChunkSize, false);
	ForkProcess("--test-shared-memory", szSharedMemoryName, szMutexName, "C", szChunkSize, false);
	ForkProcess("--test-shared-memory", szSharedMemoryName, szMutexName, "D", szChunkSize, false);
	
	for (;;)
	{
		bResult = cMutex.Lock();

		puiPosition = (unsigned int*)cSharedOwner.Touch().pvMemory;
		if (!puiPosition)
		{
			AssertNotNull(puiPosition);
		}
		uiPosition = *puiPosition;

		bResult = cMutex.Unlock();

		if (uiPosition == 16000 + sizeof(int))
		{
			break;
		}

		ThreadYield();
	}

	cHold.Stop();
	AssertTrue(cDone.Wait(4, TEST_TIMEOUT));

	cHold.Kill();
	cDone.Kill();

	pcMemory = (char*)RemapSinglePointer(cSharedOwner.Touch().pvMemory, sizeof(int));
	AssertFilledMemory(pcMemory, iChunkSize);

	cSharedOwner.Close();
	cSharedOwner.Kill();

	cMutex.Close();
	cMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemoryResizeMultiProcessRepeat(int iIterations)
{
	int i;

	for (i = 0; i < iIterations; i++)
	{
		TestSharedMemoryResizeMultiProcess();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedMemory(void)
{
	BeginTests();

	TestSharedMemoryOneProcess();
	TestSharedMemoryTwoProcesses();
	TestSharedMemoryResizeOneProcess();
	TestSharedMemoryResizeOneProcessFill();
	TestSharedMemoryResizeOneProcessResizeAndRelease();
	TestSharedMemoryResizeMultiThreadRepeat(100);
	ThreadSleep(500);
	TestSharedMemoryResizeMultiProcessRepeat(100);

	TestStatistics();
}

