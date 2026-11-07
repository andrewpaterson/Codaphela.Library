#include "BaseLib/GlobalMemory.h"
#include "StandardLib/BinaryHeap.h"
#include "TestLib/Assert.h"


struct STestHeapItem
{
	int		iKey;
	size	uiOrder;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestBinaryHeapIntLess(int* piLeft, int* piRight)
{
	return *piLeft < *piRight;
}


//////////////////////////////////////////////////////////////////////////
//
// Equal keys come out in the order they went in (as CDistToRootExpectedDists
// orders its nodes).
//
//////////////////////////////////////////////////////////////////////////
static bool TestBinaryHeapItemLess(STestHeapItem* psLeft, STestHeapItem* psRight)
{
	if (psLeft->iKey != psRight->iKey)
	{
		return psLeft->iKey < psRight->iKey;
	}
	return psLeft->uiOrder < psRight->uiOrder;
}


//////////////////////////////////////////////////////////////////////////
//
// What is pushed comes out least first, however it went in.
//
//////////////////////////////////////////////////////////////////////////
void TestBinaryHeapOrder(void)
{
	CArrayTemplate<int>	aiHeap;
	int					i;
	int					iValue;
	int					iLast;
	uint32				uiSeed;

	aiHeap.Init();
	uiSeed = 12345;
	for (i = 0; i < 200; i++)
	{
		uiSeed = uiSeed * 1103515245 + 12345;
		iValue = (int)((uiSeed >> 16) % 1000);
		BinaryHeapPush(&aiHeap, &iValue, TestBinaryHeapIntLess);
	}
	AssertSize(200, aiHeap.NumElements());

	iLast = -1;
	for (i = 0; i < 200; i++)
	{
		BinaryHeapPop(&aiHeap, &iValue, TestBinaryHeapIntLess);
		AssertTrue(iValue >= iLast);
		iLast = iValue;
	}
	AssertSize(0, aiHeap.NumElements());
	aiHeap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes and pops mixed, with ties broken by the order they went in.
//
//////////////////////////////////////////////////////////////////////////
void TestBinaryHeapMixed(void)
{
	CArrayTemplate<STestHeapItem>	asHeap;
	STestHeapItem					sItem;
	size							uiOrder;
	int								aiKeys[] = { 5, 3, 5, 1, 3, 9, 1 };
	size							i;

	asHeap.Init();
	uiOrder = 0;
	for (i = 0; i < 4; i++)
	{
		sItem.iKey = aiKeys[i];
		sItem.uiOrder = uiOrder++;
		BinaryHeapPush(&asHeap, &sItem, TestBinaryHeapItemLess);
	}

	//5(0) 3(1) 5(2) 1(3): the 1 first.
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(1, sItem.iKey);
	AssertSize(3, sItem.uiOrder);

	for (i = 4; i < 7; i++)
	{
		sItem.iKey = aiKeys[i];
		sItem.uiOrder = uiOrder++;
		BinaryHeapPush(&asHeap, &sItem, TestBinaryHeapItemLess);
	}

	//Left: 5(0) 3(1) 5(2) 3(4) 9(5) 1(6).
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(1, sItem.iKey);
	AssertSize(6, sItem.uiOrder);
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(3, sItem.iKey);
	AssertSize(1, sItem.uiOrder);
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(3, sItem.iKey);
	AssertSize(4, sItem.uiOrder);
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(5, sItem.iKey);
	AssertSize(0, sItem.uiOrder);
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(5, sItem.iKey);
	AssertSize(2, sItem.uiOrder);
	BinaryHeapPop(&asHeap, &sItem, TestBinaryHeapItemLess);
	AssertInt(9, sItem.iKey);
	AssertSize(0, asHeap.NumElements());

	asHeap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBinaryHeap(void)
{
	BeginTests();
	MemoryInit();

	TestBinaryHeapOrder();
	TestBinaryHeapMixed();

	MemoryKill();
	TestStatistics();
}
