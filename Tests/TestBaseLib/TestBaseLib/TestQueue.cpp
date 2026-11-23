#include "BaseLib/QueueTemplate.h"
#include "TestLib/Assert.h"


//An element that a queue must give its virtual function table.
class CTestQueueVirtual
{
public:
	int32	miValue;

	virtual int32 Twice(void) { return miValue * 2; }
};


//An element of a bounded history: Killed before it is dropped.
struct STestQueuePlace
{
	int32	iLine;
	bool	bKilled;

	void Kill(void) { bKilled = true; }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQueueOrder(void)
{
	CQueueTemplate<int32>	cQueue;
	int32					i;
	int32					iValue;
	bool					bResult;

	cQueue.Init();
	AssertTrue(cQueue.IsEmpty());
	AssertSize(0, cQueue.NumElements());
	AssertNull(cQueue.Front());
	AssertNull(cQueue.Back());
	AssertFalse(cQueue.Pop(&iValue));
	AssertFalse(cQueue.Pop());
	AssertFalse(cQueue.RemoveBack());

	for (i = 0; i < 100; i++)
	{
		*cQueue.Push() = i;
	}
	AssertSize(100, cQueue.NumElements());
	AssertInt(0, *cQueue.Front());
	AssertInt(99, *cQueue.Back());

	bResult = true;
	for (i = 0; i < 100; i++)
	{
		bResult &= (*cQueue.Get(0) == i);
		bResult &= cQueue.Pop(&iValue);
		bResult &= (iValue == i);
		bResult &= (cQueue.NumElements() == (size)(99 - i));
	}
	AssertTrue(bResult);
	AssertTrue(cQueue.IsEmpty());
	AssertNull(cQueue.Front());

	//Filled again after it was emptied.
	iValue = 7;
	cQueue.Push(&iValue);
	AssertSize(1, cQueue.NumElements());
	AssertInt(7, *cQueue.Front());
	AssertInt(7, *cQueue.Back());

	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pops that cross the point where the dead front is moved out: 20 pushed,
// 10 taken (the dead equal the live: not moved), then one more (moved).
//
//////////////////////////////////////////////////////////////////////////
void TestQueueCompaction(void)
{
	CQueueTemplate<int32>	cQueue;
	int32					i;
	bool					bResult;

	cQueue.Init();
	for (i = 0; i < 20; i++)
	{
		*cQueue.Push() = i;
	}
	for (i = 0; i < 10; i++)
	{
		cQueue.Pop();
	}
	AssertSize(10, cQueue.NumElements());
	AssertInt(10, *cQueue.Front());
	AssertInt(19, *cQueue.Back());

	cQueue.Pop();
	AssertSize(9, cQueue.NumElements());
	bResult = true;
	for (i = 0; i < 9; i++)
	{
		bResult &= (*cQueue.Get((size)i) == i + 11);
	}
	AssertTrue(bResult);
	AssertInt(11, *cQueue.Front());
	AssertInt(19, *cQueue.Back());

	//A small queue's front isn't moved for every pop.
	cQueue.ReInit();
	for (i = 0; i < 5; i++)
	{
		*cQueue.Push() = i;
	}
	for (i = 0; i < 4; i++)
	{
		cQueue.Pop();
	}
	AssertSize(1, cQueue.NumElements());
	AssertInt(4, *cQueue.Front());
	cQueue.Pop();
	AssertTrue(cQueue.IsEmpty());

	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes and pops interleaved, against a count of what should be there.
//
//////////////////////////////////////////////////////////////////////////
void TestQueueInterleaved(void)
{
	CQueueTemplate<int32>	cQueue;
	int32					iNext;
	int32					iFirst;
	int32					iStep;
	int32					i;
	bool					bResult;

	cQueue.Init();
	iNext = 0;
	iFirst = 0;
	bResult = true;
	for (iStep = 0; iStep < 1000; iStep++)
	{
		for (i = 0; i < 3; i++)
		{
			*cQueue.Push() = iNext++;
		}
		for (i = 0; i < ((iStep % 7 == 0) ? 4 : 2); i++)
		{
			if (!cQueue.IsEmpty())
			{
				bResult &= (*cQueue.Front() == iFirst);
				cQueue.Pop();
				iFirst++;
			}
		}
		bResult &= (cQueue.NumElements() == (size)(iNext - iFirst));
		if (!cQueue.IsEmpty())
		{
			bResult &= (*cQueue.Get(cQueue.NumElements() - 1) == iNext - 1);
			bResult &= (*cQueue.Back() == iNext - 1);
		}
	}
	AssertTrue(bResult);
	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A bounded history: the oldest Killed and dropped past a maximum; going
// back takes from the back; a new place cuts the redo end.
//
//////////////////////////////////////////////////////////////////////////
void TestQueueHistory(void)
{
	CQueueTemplate<STestQueuePlace>	cHistory;
	STestQueuePlace*				psPlace;
	int32							i;
	bool							bResult;

	cHistory.Init();
	for (i = 0; i < 30; i++)
	{
		while (cHistory.NumElements() >= 10)
		{
			cHistory.Front()->Kill();
			cHistory.Pop();
		}
		psPlace = cHistory.Push();
		psPlace->bKilled = false;
		psPlace->iLine = i;
	}
	AssertSize(10, cHistory.NumElements());
	bResult = true;
	for (i = 0; i < 10; i++)
	{
		bResult &= (cHistory.Get((size)i)->iLine == 20 + i);
		bResult &= !cHistory.Get((size)i)->bKilled;
	}
	AssertTrue(bResult);

	AssertTrue(cHistory.RemoveBack());
	AssertInt(28, cHistory.Back()->iLine);
	AssertSize(9, cHistory.NumElements());

	//The front isn't at the start of the array here.
	cHistory.RemoveFrom(5);
	AssertSize(5, cHistory.NumElements());
	AssertInt(20, cHistory.Front()->iLine);
	AssertInt(24, cHistory.Back()->iLine);
	cHistory.RemoveFrom(5);
	AssertSize(5, cHistory.NumElements());
	cHistory.RemoveFrom(0);
	AssertTrue(cHistory.IsEmpty());

	cHistory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQueueVirtual(void)
{
	CQueueTemplate<CTestQueueVirtual>	cQueue;
	CTestQueueVirtual*					pcElement;

	cQueue.Init();
	pcElement = cQueue.Push();
	pcElement->miValue = 21;
	AssertInt(42, cQueue.Front()->Twice());
	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQueueLarge(void)
{
	CQueueTemplate<int32, CArrayLargeBlock>	cQueue;
	int32									i;

	cQueue.Init();
	for (i = 0; i < 1000; i++)
	{
		*cQueue.Push() = i;
	}
	for (i = 0; i < 600; i++)
	{
		cQueue.Pop();
	}
	AssertLSize(400, cQueue.NumElements());
	AssertInt(600, *cQueue.Front());
	AssertInt(999, *cQueue.Back());
	cQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQueue(void)
{
	BeginTests();

	TestQueueOrder();
	TestQueueCompaction();
	TestQueueInterleaved();
	TestQueueHistory();
	TestQueueVirtual();
	TestQueueLarge();

	TestStatistics();
}

