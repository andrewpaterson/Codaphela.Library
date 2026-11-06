#include <stddef.h>
#include <string.h>
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
int32 GetTestRedBlackTreeDepth(SRedBlackNode* psNode)
{
	int32	iLeft;
	int32	iRight;

	if (psNode == NULL)
	{
		return 0;
	}
	iLeft = GetTestRedBlackTreeDepth(psNode->psLeft);
	iRight = GetTestRedBlackTreeDepth(psNode->psRight);
	if (iLeft > iRight)
	{
		return iLeft + 1;
	}
	return iRight + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A red-black tree of n elements is never deeper than 2 * log2(n + 1).
//
//////////////////////////////////////////////////////////////////////////
int32 GetTestRedBlackTreeMaxDepth(size iNumElements)
{
	int32	iLog;
	size	iPower;

	iLog = 0;
	iPower = 1;
	while (iPower < iNumElements + 1)
	{
		iPower *= 2;
		iLog++;
	}
	return iLog * 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeBalanced(void)
{
	CTestRedBlackTree	cTree;
	int64				iTime;
	int32				i;

	InitTestRedBlackTree(&cTree);
	AssertInt((int32)0, GetTestRedBlackTreeDepth(cTree.TestGetRoot()));

	for (i = 0; i < 4095; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	AssertTrue(cTree.IsValid());
	AssertTrue(GetTestRedBlackTreeDepth(cTree.TestGetRoot()) <= GetTestRedBlackTreeMaxDepth(cTree.NumElements()));

	for (i = 0; i < 4095; i += 3)
	{
		iTime = i;
		cTree.Remove(&iTime);
	}
	AssertTrue(cTree.IsValid());
	AssertTrue(GetTestRedBlackTreeDepth(cTree.TestGetRoot()) <= GetTestRedBlackTreeMaxDepth(cTree.NumElements()));
	cTree.Kill();

	//Keys alternating in from both ends.
	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 1000; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
		AddTestRedBlackTreeItem(&cTree, 1999 - i, 1999 - i);
	}
	AssertTrue(cTree.IsValid());
	AssertSize(2000, cTree.NumElements());
	AssertTrue(GetTestRedBlackTreeDepth(cTree.TestGetRoot()) <= GetTestRedBlackTreeMaxDepth(cTree.NumElements()));
	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Checks every query against a plain array of which keys are present.
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeNeighbours(void)
{
	CTestRedBlackTree		cTree;
	bool					abPresent[200];
	STestRedBlackTreeItem*	psItem;
	STestRedBlackTreeItem*	psExpected;
	uint32					uiSeed;
	int64					iTime;
	int32					i;
	int32					j;
	int32					iRound;
	int32					iBefore;
	int32					iAfter;
	size					iCount;
	bool					bMatch;

	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 200; i++)
	{
		abPresent[i] = false;
	}

	bMatch = true;
	uiSeed = 777;
	for (iRound = 0; iRound < 40; iRound++)
	{
		for (j = 0; j < 50; j++)
		{
			uiSeed = uiSeed * 1103515245 + 12345;
			i = (int32)((uiSeed >> 8) % 200);
			iTime = i;
			if (abPresent[i])
			{
				cTree.Remove(&iTime);
			}
			else
			{
				AddTestRedBlackTreeItem(&cTree, i, i);
			}
			abPresent[i] = !abPresent[i];
		}

		//Keys -1 and 200 are outside every element.
		for (i = -1; i <= 200; i++)
		{
			iBefore = i;
			while ((iBefore >= 0) && ((iBefore >= 200) || !abPresent[iBefore]))
			{
				iBefore--;
			}
			iAfter = i;
			while ((iAfter < 200) && ((iAfter < 0) || !abPresent[iAfter]))
			{
				iAfter++;
			}

			iTime = i;
			psItem = cTree.GetOnOrBefore(&iTime);
			if (iBefore < 0)
			{
				bMatch &= (psItem == NULL);
			}
			else
			{
				bMatch &= (psItem != NULL) && (psItem->iValue == iBefore);
			}

			psItem = cTree.GetOnOrAfter(&iTime);
			if (iAfter >= 200)
			{
				bMatch &= (psItem == NULL);
			}
			else
			{
				bMatch &= (psItem != NULL) && (psItem->iValue == iAfter);
			}

			bMatch &= cTree.Contains(&iTime) == ((i >= 0) && (i < 200) && abPresent[i]);
		}

		//Forwards with GetNext and backwards with GetPrevious.
		iCount = 0;
		psExpected = NULL;
		psItem = cTree.GetFirst();
		for (i = 0; i < 200; i++)
		{
			if (abPresent[i])
			{
				bMatch &= (psItem != NULL) && (psItem->iValue == i);
				if (psItem)
				{
					bMatch &= cTree.GetPrevious(psItem) == psExpected;
					psExpected = psItem;
					psItem = cTree.GetNext(psItem);
				}
				iCount++;
			}
		}
		bMatch &= (psItem == NULL);
		bMatch &= (psExpected == cTree.GetLast());
		bMatch &= (iCount == cTree.NumElements());
		bMatch &= cTree.IsValid();
	}
	AssertTrue(bMatch);

	cTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Removing elements other than the current one doesn't disturb an iteration, like the timeline removing events while
// it runs others.
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeRemoveWhileIterating(void)
{
	CTestRedBlackTree		cTree;
	SRedBlackTreeIterator	sIter;
	STestRedBlackTreeItem*	psItem;
	int64					iTime;
	int64					iStart;
	int64					iEnd;
	int32					i;
	int32					aiVisited[20];
	int32					iNumVisited;

	//Each element removes the element after next and the one before it.
	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 20; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}

	iNumVisited = 0;
	psItem = cTree.StartIteration(&sIter);
	while (psItem)
	{
		aiVisited[iNumVisited] = psItem->iValue;
		iNumVisited++;

		iTime = psItem->iTime + 2;
		cTree.Remove(&iTime);
		iTime = psItem->iTime - 1;
		cTree.Remove(&iTime);
		AssertTrue(cTree.IsValid());

		psItem = cTree.Iterate(&sIter);
	}
	//0 removes 2, 1 removes 3 and 0, 4 removes 6 and 3, 5 removes 7 and 4, ...
	AssertInt((int32)10, iNumVisited);
	AssertInt((int32)0, aiVisited[0]);
	AssertInt((int32)1, aiVisited[1]);
	AssertInt((int32)4, aiVisited[2]);
	AssertInt((int32)5, aiVisited[3]);
	AssertInt((int32)8, aiVisited[4]);
	AssertInt((int32)9, aiVisited[5]);
	AssertInt((int32)12, aiVisited[6]);
	AssertInt((int32)13, aiVisited[7]);
	AssertInt((int32)16, aiVisited[8]);
	AssertInt((int32)17, aiVisited[9]);
	AssertSize(5, cTree.NumElements());
	AssertLong(1, cTree.GetFirst()->iTime);
	AssertLong(5, cTree.GetNext(cTree.GetFirst())->iTime);
	AssertLong(17, cTree.GetLast()->iTime);
	cTree.Kill();

	//Removing the last element of the whole tree mid iteration.
	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 10; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	iNumVisited = 0;
	psItem = cTree.StartIteration(&sIter);
	while (psItem)
	{
		iNumVisited++;
		if (psItem->iTime == 3)
		{
			iTime = 9;
			cTree.Remove(&iTime);
		}
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)9, iNumVisited);
	cTree.Kill();

	//Removing the last element of a range mid iteration.
	InitTestRedBlackTree(&cTree);
	for (i = 0; i < 10; i++)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	iStart = 2;
	iEnd = 5;
	iNumVisited = 0;
	psItem = cTree.StartIteration(&sIter, &iStart, &iEnd);
	while (psItem && (iNumVisited < 20))
	{
		aiVisited[iNumVisited] = psItem->iValue;
		iNumVisited++;
		if (psItem->iTime == 2)
		{
			iTime = 5;
			cTree.Remove(&iTime);
		}
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)3, iNumVisited);
	AssertInt((int32)2, aiVisited[0]);
	AssertInt((int32)3, aiVisited[1]);
	AssertInt((int32)4, aiVisited[2]);
	cTree.Kill();

	//Adding elements mid iteration, one in range and one after it, after removing the last in range.
	InitTestRedBlackTree(&cTree);
	for (i = 0; i <= 10; i += 2)
	{
		AddTestRedBlackTreeItem(&cTree, i, i);
	}
	iStart = 2;
	iEnd = 8;
	iNumVisited = 0;
	psItem = cTree.StartIteration(&sIter, &iStart, &iEnd);
	while (psItem && (iNumVisited < 20))
	{
		aiVisited[iNumVisited] = psItem->iValue;
		iNumVisited++;
		if (psItem->iTime == 2)
		{
			iTime = 8;
			cTree.Remove(&iTime);
			AddTestRedBlackTreeItem(&cTree, 9, 9);
			AddTestRedBlackTreeItem(&cTree, 7, 7);
		}
		psItem = cTree.Iterate(&sIter);
	}
	AssertInt((int32)4, iNumVisited);
	AssertInt((int32)2, aiVisited[0]);
	AssertInt((int32)4, aiVisited[1]);
	AssertInt((int32)6, aiVisited[2]);
	AssertInt((int32)7, aiVisited[3]);
	AssertNull(cTree.Iterate(&sIter));
	cTree.Kill();
}


struct STestRedBlackTreeName
{
	int32	iId;
	char	szName[20];
	float	fWeight;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareTestRedBlackTreeName(const void* pvLeft, const void* pvRight)
{
	return strcmp((char*)pvLeft, (char*)pvRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STestRedBlackTreeName* AddTestRedBlackTreeName(CRedBlackTreeTemplate<STestRedBlackTreeName>* pcTree, int32 iId, const char* szName, float fWeight)
{
	STestRedBlackTreeName	sName;

	memset(&sName, 0, sizeof(STestRedBlackTreeName));
	sName.iId = iId;
	strcpy(sName.szName, szName);
	sName.fWeight = fWeight;
	return pcTree->Put(&sName);
}


//////////////////////////////////////////////////////////////////////////
//
// A key in the middle of a larger element, compared by string.
//
//////////////////////////////////////////////////////////////////////////
void TestRedBlackTreeStringKey(void)
{
	CRedBlackTreeTemplate<STestRedBlackTreeName>	cTree;
	SRedBlackTreeIterator							sIter;
	STestRedBlackTreeName*							psName;
	STestRedBlackTreeName*							psDelta;

	cTree.Init(offsetof(STestRedBlackTreeName, szName), CompareTestRedBlackTreeName);

	AddTestRedBlackTreeName(&cTree, 1, "Delta", 1.5f);
	AddTestRedBlackTreeName(&cTree, 2, "Alpha", 2.5f);
	AddTestRedBlackTreeName(&cTree, 3, "Echo", 3.5f);
	AddTestRedBlackTreeName(&cTree, 4, "Charlie", 4.5f);
	AddTestRedBlackTreeName(&cTree, 5, "Bravo", 5.5f);
	AssertTrue(cTree.IsValid());
	AssertSize(5, cTree.NumElements());

	psName = cTree.StartIteration(&sIter);
	AssertString("Alpha", psName->szName);
	AssertInt((int32)2, psName->iId);
	AssertFloat(2.5f, psName->fWeight, 2);
	psName = cTree.Iterate(&sIter);
	AssertString("Bravo", psName->szName);
	psName = cTree.Iterate(&sIter);
	AssertString("Charlie", psName->szName);
	psName = cTree.Iterate(&sIter);
	AssertString("Delta", psName->szName);
	psName = cTree.Iterate(&sIter);
	AssertString("Echo", psName->szName);
	AssertNull(cTree.Iterate(&sIter));

	psDelta = cTree.Get((char*)"Delta");
	AssertInt((int32)1, psDelta->iId);
	AssertNull(cTree.Get((char*)"delta"));
	AssertString("Charlie", cTree.GetOnOrBefore((char*)"Cz")->szName);
	AssertString("Delta", cTree.GetOnOrAfter((char*)"Cz")->szName);

	//Put replaces the whole element in place.
	AssertPointer(psDelta, AddTestRedBlackTreeName(&cTree, 9, "Delta", 9.5f));
	AssertInt((int32)9, psDelta->iId);
	AssertFloat(9.5f, psDelta->fWeight, 2);
	AssertSize(5, cTree.NumElements());

	AssertTrue(cTree.Remove((char*)"Alpha"));
	AssertTrue(cTree.Remove((char*)"Echo"));
	AssertFalse(cTree.Remove((char*)"Alpha"));
	AssertTrue(cTree.IsValid());
	AssertString("Bravo", cTree.GetFirst()->szName);
	AssertString("Delta", cTree.GetLast()->szName);
	AssertPointer(psDelta, cTree.Get((char*)"Delta"));

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
	TestRedBlackTreeBalanced();
	TestRedBlackTreeNeighbours();
	TestRedBlackTreeRemoveWhileIterating();
	TestRedBlackTreeStringKey();

	TestStatistics();
}

