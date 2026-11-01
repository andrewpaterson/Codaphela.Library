#include "BaseLib/DebugOutput.h"
#include "BaseLib/Logger.h"
#include "ThreadLib/ResizableSharedMemory.h"
#include "ThreadLib/InterProcessDone.h"
#include "SharedMemoryFill.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestSharedMemoryProcessReverse(char* szSharedMemoryName, char* szMemorySize)
{
	CResizableSharedMemory	cSharedClient;
	unsigned char*	pcClient;
	int				i;
	int				iMemorySize;
	
	iMemorySize = atoi(szMemorySize);
	cSharedClient.Init(szSharedMemoryName);

	pcClient = (unsigned char*)cSharedClient.Touch().pvMemory;
	if (pcClient == NULL)
	{
		cSharedClient.Close();
		return 1;
	}

	for (i = 0; i < iMemorySize; i++)
	{
		pcClient[i] = 255 - pcClient[i];
	}

	cSharedClient.Close();
	cSharedClient.Kill();

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestSharedMemoryProcessFill(char* szSharedMemoryName, char* szMutexName, char* szFillChar, char* szChunkSize)
{
	CSharedMemoryFill	cFill;
	
	cFill.Init(szSharedMemoryName, szMutexName, szFillChar, atoi(szChunkSize));
	cFill.Run();

	return cFill.miResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestSharedMemoryProcessMain(int argc, char* argv[])
{
	if (argc == 2)
	{
		return TestSharedMemoryProcessReverse(argv[0], argv[1]);
	}
	else if (argc == 4)
	{
		return TestSharedMemoryProcessFill(argv[0], argv[1], argv[2], argv[3]);
	}
	else
	{
		return -1;
	}
}

