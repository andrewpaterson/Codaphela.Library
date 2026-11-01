#include <stddef.h>
#include "TestLib/Assert.h"
#include "BaseLib/RedBlackTree.h"


struct STestRedBlackTreeItem
{
	int32	iValue;
	int64	iTime;
};


typedef CRedBlackTreeTemplate<STestRedBlackTreeItem> CTestRedBlackTree;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareTestRedBlackTreeTime(const void* pvLeft, const void* pvRight)
{
	int64	iLeft;
	int64	iRight;

	iLeft = *((int64*)pvLeft);
	iRight = *((int64*)pvRight);
	if (iLeft < iRight)
	{
		return -1;
	}
	else if (iLeft > iRight)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitTestRedBlackTree(CTestRedBlackTree* pcTree)
{
	pcTree->Init(offsetof(STestRedBlackTreeItem, iTime), CompareTestRedBlackTreeTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STestRedBlackTreeItem* AddTestRedBlackTreeItem(CTestRedBlackTree* pcTree, int64 iTime, int32 iValue)
{
	STestRedBlackTreeItem	sItem;

	sItem.iTime = iTime;
	sItem.iValue = iValue;
	return pcTree->Add(&sItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeEmpty(void)
{
	CTestRedBlackTree		cTree;
	SRedBlackTreeIterator	sIter;
	int64					iTime;

	InitTestRedBlackTree(&cTree);

	iTime = 5;
	AssertTrue(cTree.IsEmpty());
	AssertSize(0, cTree.NumElements());
	AssertNull(cTree.Get(&iTime));
	AssertNull(cTree.GetOnOrBefore(&iTime));
	AssertNull(cTree.GetOnOrAfter(&iTime));
	AssertNull(cTree.GetFirst());
	AssertNull(cTree.GetLast());
	AssertNull(cTree.StartIteration(&sIter));
	AssertNull(cTree.Iterate(&sIter));
	AssertNull(cTree.StartIteration(&sIter, &iTime, NULL));
	AssertFalse(cTree.Contains(&iTime));
	AssertFalse(cTree.Remove(&iTime));
	AssertTrue(cTree.IsValid());

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeAdd(void)
{
	CTestRedBlackTree		cTree;
	SRedBlackTreeIterator	sIter;
	STestRedBlackTreeItem*	psItem;
	int64					iTime;
	int32					i;

	InitTestRedBlackTree(&cTree);

	for (i = 0; i < 1000; i++)
	{
		psItem = AddTestRedBlackTreeItem(&cTree, (int64)i * 3, i);
		AssertNotNull(psItem);
		AssertLong((int64)i * 3, psItem->iTime);
		AssertInt(i, psItem->iValue);
		if (i < 100)
		{
			AssertTrue(cTree.IsValid());
		}
	}
	AssertTrue(cTree.IsValid());
	AssertSize(1000, cTree.NumElements());
	AssertFalse(cTree.IsEmpty());

	AssertNull(AddTestRedBlackTreeItem(&cTree, 30, -1));
	AssertSize(1000, cTree.NumElements());
	iTime = 30;
	AssertInt((int32)10, cTree.Get(&iTime)->iValue);

	iTime = 31;
	AssertNull(cTree.Get(&iTime));
	AssertFalse(cTree.Contains(&iTime));
	iTime = 33;
	AssertTrue(cTree.Contains(&iTime));

	AssertLong(0, cTree.GetFirst()->iTime);
	AssertLong(2997, cTree.GetLast()->iTime);

	i = 0;
	psItem = cTree.StartIteration(&sIter);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i++;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)1000, i);

	cTree.Kill();

	InitTestRedBlackTree(&cTree);
	for (i = 1000; i > 0; i--)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	AssertTrue(cTree.IsValid());
	AssertLong(1, cTree.GetFirst()->iTime);
	AssertLong(1000, cTree.GetLast()->iTime);
	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeFind(void)
{
	CTestRedBlackTree	cTree;
	int64				iTime;

	InitTestRedBlackTree(&cTree);
	AddTestRedBlackTreeItem(&cTree, 20, 2);
	AddTestRedBlackTreeItem(&cTree, 10, 1);
	AddTestRedBlackTreeItem(&cTree, 30, 3);

	iTime = 5;
	AssertNull(cTree.GetOnOrBefore(&iTime));
	AssertInt((int32)1, cTree.GetOnOrAfter(&iTime)->iValue);
	iTime = 10;
	AssertInt((int32)1, cTree.GetOnOrBefore(&iTime)->iValue);
	AssertInt((int32)1, cTree.GetOnOrAfter(&iTime)->iValue);
	iTime = 15;
	AssertInt((int32)1, cTree.GetOnOrBefore(&iTime)->iValue);
	AssertInt((int32)2, cTree.GetOnOrAfter(&iTime)->iValue);
	iTime = 29;
	AssertInt((int32)2, cTree.GetOnOrBefore(&iTime)->iValue);
	AssertInt((int32)3, cTree.GetOnOrAfter(&iTime)->iValue);
	iTime = 35;
	AssertInt((int32)3, cTree.GetOnOrBefore(&iTime)->iValue);
	AssertNull(cTree.GetOnOrAfter(&iTime));

	iTime = 20;
	AssertInt((int32)1, cTree.GetPrevious(cTree.Get(&iTime))->iValue);
	AssertInt((int32)3, cTree.GetNext(cTree.Get(&iTime))->iValue);
	AssertNull(cTree.GetPrevious(cTree.GetFirst()));
	AssertNull(cTree.GetNext(cTree.GetLast()));

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeIterateRange(void)
{
	CTestRedBlackTree		cTree;
	SRedBlackTreeIterator	sIter;
	STestRedBlackTreeItem*	psItem;
	int64					iStart;
	int64					iEnd;
	int32					i;

	InitTestRedBlackTree(&cTree);
	for (i = 1; i <= 10; i++)
	{
		AddTestRedBlackTreeItem(&cTree, (int64)i * 10, i);
	}

	iStart = 25;
	iEnd = 65;
	i = 3;
	psItem = cTree.StartIteration(&sIter, &iStart, &iEnd);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i++;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)7, i);

	iStart = 30;
	iEnd = 60;
	i = 3;
	psItem = cTree.StartIteration(&sIter, &iStart, &iEnd);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i++;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)7, i);

	iStart = 75;
	i = 8;
	psItem = cTree.StartIteration(&sIter, &iStart, NULL);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i++;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)11, i);

	iEnd = 30;
	i = 1;
	psItem = cTree.StartIteration(&sIter, NULL, &iEnd);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i++;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)4, i);

	iStart = 31;
	iEnd = 39;
	AssertNull(cTree.StartIteration(&sIter, &iStart, &iEnd));
	AssertNull(cTree.Iterate(&sIter));

	iStart = 60;
	iEnd = 30;
	AssertNull(cTree.StartIteration(&sIter, &iStart, &iEnd));

	iStart = 101;
	AssertNull(cTree.StartIteration(&sIter, &iStart, NULL));
	iEnd = 9;
	AssertNull(cTree.StartIteration(&sIter, NULL, &iEnd));

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreePut(void)
{
	CTestRedBlackTree		cTree;
	STestRedBlackTreeItem	sItem;
	STestRedBlackTreeItem*	psItem;
	int64					iTime;

	InitTestRedBlackTree(&cTree);
	psItem = AddTestRedBlackTreeItem(&cTree, 7, 70);

	sItem.iTime = 7;
	sItem.iValue = 71;
	AssertPointer(psItem, cTree.Put(&sItem));
	AssertInt((int32)71, psItem->iValue);
	AssertSize(1, cTree.NumElements());

	sItem.iTime = 8;
	sItem.iValue = 80;
	psItem = cTree.Put(&sItem);
	AssertInt((int32)80, psItem->iValue);
	AssertSize(2, cTree.NumElements());
	iTime = 8;
	AssertPointer(psItem, cTree.Get(&iTime));

	cTree.RemoveAll();
	AssertSize(0, cTree.NumElements());
	AssertNull(cTree.GetFirst());
	AssertNotNull(AddTestRedBlackTreeItem(&cTree, 7, 70));
	AssertTrue(cTree.IsValid());

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeRemove(void)
{
	CTestRedBlackTree		cTree;
	SRedBlackTreeIterator	sIter;
	STestRedBlackTreeItem*	psItem;
	int64					iTime;
	int32					i;

	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 100; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}

	for (i = 0; i < 100; i += 2)
	{
		iTime = i;
		AssertTrue(cTree.Remove(&iTime));
		AssertTrue(cTree.IsValid());
		AssertFalse(cTree.Remove(&iTime));
	}
	AssertSize(50, cTree.NumElements());

	i = 1;
	psItem = cTree.StartIteration(&sIter);
	while (psItem)
	{
		AssertInt(i, psItem->iValue);
		i += 2;
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)101, i);

	while (!cTree.IsEmpty())
	{
		cTree.RemoveElement(cTree.GetFirst());
		AssertTrue(cTree.IsValid());
	}
	AssertNull(cTree.GetFirst());

	for (i = 0; i < 100; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	while (!cTree.IsEmpty())
	{
		cTree.RemoveElement(cTree.GetLast());
		AssertTrue(cTree.IsValid());
	}

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeRandom(void)
{
	CTestRedBlackTree		cTree;
	STestRedBlackTreeItem*	apsItems[500];
	STestRedBlackTreeItem*	psItem;
	uint32					uiSeed;
	int32					iKey;
	int64					iTime;
	int32					i;
	size					iCount;
	bool					bValid;
	bool					bPointers;

	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 500; i++)
	{
		apsItems[i] = NULL;
	}

	iCount = 0;
	bValid = true;
	bPointers = true;
	uiSeed = 12345;
	for (i = 0; i < 20000; i++)
	{
		uiSeed = uiSeed * 1103515245 + 12345;
		iKey = (int32)((uiSeed >> 8) % 500);
		iTime = iKey;
		if (apsItems[iKey])
		{
			if (!cTree.Remove(&iTime))
			{
				bValid = false;
			}
			apsItems[iKey] = NULL;
			iCount--;
		}
		else
		{
			apsItems[iKey] = AddTestRedBlackTreeItem(&cTree, iKey, iKey * 2);
			if (apsItems[iKey] == NULL)
			{
				bValid = false;
			}
			iCount++;
		}

		if ((i % 97) == 0)
		{
			if (!cTree.IsValid() || (cTree.NumElements() != iCount))
			{
				bValid = false;
			}
		}
	}
	AssertTrue(bValid);
	AssertTrue(cTree.IsValid());
	AssertSize(iCount, cTree.NumElements());

	for (i = 0; i < 500; i++)
	{
		iTime = i;
		psItem = cTree.Get(&iTime);
		if (psItem != apsItems[i])
		{
			bPointers = false;
		}
		if (psItem && (psItem->iValue != i * 2))
		{
			bPointers = false;
		}
	}
	AssertTrue(bPointers);

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTree(void)
{
	BeginTests();

	TestRedBlackTreeEmpty();
	TestRedBlackTreeAdd();
	TestRedBlackTreeFind();
	TestRedBlackTreeIterateRange();
	TestRedBlackTreePut();
	TestRedBlackTreeRemove();
	TestRedBlackTreeRandom();

	TestStatistics();
}

