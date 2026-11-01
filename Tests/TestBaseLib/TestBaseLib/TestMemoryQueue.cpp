#include "BaseLib/MemoryQueue.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FillQueueElement(void* pvData, size uiSize, char c)
{
	if (pvData && uiSize > 1)
	{
		memset(pvData, c, uiSize - 1);
		((char*)pvData)[uiSize - 1] = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertQueueElement(CMemoryQueue* pcQueue, void* pvData, size iDataSize, char cExpected)
{
	char*						szData;
	CChars						szExpected;


	AssertLSize(iDataSize, pcQueue->GetSize(pvData));

	szData = (char*)pvData;
	szExpected.Init();
	szExpected.Append(cExpected, iDataSize - 1);
	AssertString(szExpected.Text(), szData);
	szExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* PushElement(CMemoryQueue* pcQueue, size uiSize, char c)
{
	void*	pv;

	pv = pcQueue->Push(uiSize);
	FillQueueElement(pv, uiSize, c);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryQueuePush(void)
{
	CMemoryQueue	cQueue;
	void*			pvQueue;
	void*			pvTemp;
	void*			pvC;
	void*			pvE;
	void*			pvF;
	lsize			uiDataSize;
	void*			pv;
	char			c;

	pvQueue = malloc(128 + sizeof(SCircularMemoryList));
	cQueue.Init(pvQueue, 128 + sizeof(SCircularMemoryList));

	AssertLSize(0, cQueue.NumElements());

	PushElement(&cQueue, 24, 'A');
	PushElement(&cQueue, 20, 'B');
	AssertLSize(2, cQueue.NumElements());
	AssertLSize(44 + 2 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());

	pv = cQueue.Peek(&uiDataSize);
	AssertQueueElement(&cQueue, pv, 24, 'A');
	cQueue.Drop(pv);

	AssertLSize(1, cQueue.NumElements());

	pv = cQueue.Peek(&uiDataSize);
	AssertQueueElement(&cQueue, pv, 20, 'B');
	cQueue.Drop(pv);

	AssertTrue(cQueue.IsEmpty());
	Pass();

	PushElement(&cQueue, 32, 'A');
	PushElement(&cQueue, 32, 'B');
	AssertLSize(2, cQueue.NumElements());
	AssertLSize(64 + 2 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());

	pvC = PushElement(&cQueue, 32, 'C');
	AssertNull(pvC);
	AssertLSize(2, cQueue.NumElements());
	AssertLSize(64 + 2 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());
	Pass();

	cQueue.ValidateCache();

	pvTemp = malloc(196 + 5 * sizeof(SMemoryCacheDescriptor));
	cQueue.Remap(pvTemp, 196 + 5 * sizeof(SMemoryCacheDescriptor));
	free(pvQueue);
	pvQueue = pvTemp;

	cQueue.ValidateCache();

	pvC = PushElement(&cQueue, 32, 'C');
	AssertNotNull(pvC);
	AssertLSize(3, cQueue.NumElements());
	AssertLSize(96 + 3 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());
	Pass();

	PushElement(&cQueue, 32, 'D');
	pvE = PushElement(&cQueue, 32, 'E');
	AssertNotNull(pvE);
	AssertLSize(5, cQueue.NumElements());
	AssertLSize(160 + 5 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());
	Pass();

	pvF = PushElement(&cQueue, 32, 'F');
	AssertNull(pvF);
	AssertLSize(5, cQueue.NumElements());
	AssertLSize(160 + 5 * sizeof(SMemoryCacheDescriptor), cQueue.GetAllocatedSize());
	Pass();

	cQueue.ValidateCache();

	for (c = 'A'; c < 'F'; c++)
	{
		pv = cQueue.Peek(&uiDataSize);
		AssertQueueElement(&cQueue, pv, 32, c);
		cQueue.Drop(pv);

		cQueue.ValidateCache();
	}
	pv = cQueue.Peek(&uiDataSize);
	AssertNull(pv);

	AssertLSize(0, cQueue.NumElements());
	AssertLSize(0, cQueue.GetAllocatedSize());
	AssertNull(cQueue.GetFirst());
	AssertNull(cQueue.GetLast());

	cQueue.Kill();
	free(pvQueue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryQueuePeek(void)
{
	CMemoryQueue	cQueue;
	void*			pvQueue;
	char			c;
	void*			pv;
	lsize			uiDataSize;

	//128 bytes in a 32 bit build; room for three elements and their descriptors.
	pvQueue = malloc(92 + 3 * sizeof(SMemoryCacheDescriptor) + sizeof(SCircularMemoryList));
	cQueue.Init(pvQueue, 92 + 3 * sizeof(SMemoryCacheDescriptor) + sizeof(SCircularMemoryList));

	AssertLSize(0, cQueue.NumElements());

	PushElement(&cQueue, 28, 'A');
	PushElement(&cQueue, 20, 'B');

	for (c = 'C'; c <= 'Z'; c++)
	{
		PushElement(&cQueue, 20 + (c % 3) * 4, c);

		pv = cQueue.Peek(&uiDataSize);
		AssertQueueElement(&cQueue, pv, 20 + ((c-2) % 3) * 4, c-2);
		cQueue.Drop(pv);

		cQueue.ValidateCache();
	}

	AssertLSize(2, cQueue.NumElements());

	cQueue.Kill();
	free(pvQueue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryQueueRemap(void)
{
	CMemoryQueue	cQueue;
	void*			pvQueue;
	void*			pvA;
	char			c;
	size			uiQueueSize;
	void*			pv;
	int				iExpectedCount;
	size			uiExpectedAllocatedSize;
	int				i;
	size			uiElementSize;

	//Every element is 20 bytes plus its descriptor (32 bytes in a 32 bit build).
	uiElementSize = 20 + sizeof(SMemoryCacheDescriptor);

	pvQueue = malloc(8 KB);
	cQueue.Init(pvQueue, uiElementSize + sizeof(SCircularMemoryList));

	pvA = PushElement(&cQueue, 20, 'A');
	cQueue.Remap(pvQueue, (lsize)3 * uiElementSize + sizeof(SCircularMemoryList));
	AssertTrue(cQueue.ValidateCache());
	AssertLSize(1, cQueue.NumElements());

	PushElement(&cQueue, 20, 'B');
	AssertTrue(cQueue.ValidateCache());
	AssertLSize(2, cQueue.NumElements());

	cQueue.Remap(pvQueue, (lsize)4 * uiElementSize + sizeof(SCircularMemoryList));
	AssertTrue(cQueue.ValidateCache());
	AssertLSize(2, cQueue.NumElements());

	cQueue.Drop(pvA);
	PushElement(&cQueue, 20, 'C');
	cQueue.Remap(pvQueue, (lsize)5 * uiElementSize + sizeof(SCircularMemoryList));

	PushElement(&cQueue, 20, 'D');
	PushElement(&cQueue, 20, 'E');
	PushElement(&cQueue, 20, 'F');
	AssertLSize(5, cQueue.NumElements());
	Pass();

	cQueue.Remap(pvQueue, (lsize)8 * uiElementSize + sizeof(SCircularMemoryList));
	cQueue.ValidateCache();
	AssertLSize(5, cQueue.NumElements());
	AssertLSize(8 * uiElementSize, cQueue.GetCacheSize());
	AssertLSize(5 * uiElementSize, cQueue.GetAllocatedSize());

	iExpectedCount = 5;
	uiExpectedAllocatedSize = (lsize)5 * uiElementSize;
	c = 'F' + 1;
	uiQueueSize = (lsize)9 * uiElementSize;
	for (;;)
	{
		cQueue.Pop();
		if (!PushElement(&cQueue, 20, c))
		{
			break;
		}
		if (!PushElement(&cQueue, 20, c + 1))
		{
			break;
		}
		AssertTrue(cQueue.ValidateCache());
		uiExpectedAllocatedSize += uiElementSize;
		iExpectedCount++;

		cQueue.Remap(pvQueue, uiQueueSize + sizeof(SCircularMemoryList));
		AssertTrue(cQueue.ValidateCache());

		AssertLSize(iExpectedCount, cQueue.NumElements());
		AssertLSize(uiExpectedAllocatedSize, cQueue.GetAllocatedSize());

		c = c + 2;
		uiQueueSize += uiElementSize / 2;
	}

	AssertLSize(8 * uiElementSize + 7 * (uiElementSize / 2), cQueue.GetCacheSize());
	AssertLSize(11, cQueue.NumElements());

	for (i = 0; i < 11; i++)
	{
		pv = cQueue.Peek(NULL);
		AssertQueueElement(&cQueue, pv, 20, 'I' + (char)i);
		cQueue.Drop(pv);

		AssertTrue(cQueue.ValidateCache());
	}

	pv = cQueue.Peek(NULL);
	AssertNull(pv);

	cQueue.Kill();
	free(pvQueue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryQueue(void)
{
	BeginTests();

	TestMemoryQueuePush();
	TestMemoryQueuePeek();
	TestMemoryQueueRemap();

	TestStatistics();
}

