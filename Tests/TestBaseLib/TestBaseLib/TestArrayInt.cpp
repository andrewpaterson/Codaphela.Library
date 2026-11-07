#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayUint.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntAddFind(void)
{
	CArrayUint	cArray;

	cArray.Init();
	cArray.AddList(-1, 1, 2, 3, 4, -1);

	cArray.AddIfUnique(5);

	AssertSize(5, cArray.NumElements());
	AssertInt(1, cArray[0]);
	AssertInt(2, cArray[1]);
	AssertInt(3, cArray[2]);
	AssertInt(4, cArray[3]);
	AssertInt(5, cArray[4]);

	cArray.AddIfUnique(5);

	AssertSize(5, cArray.NumElements());
	AssertInt(1, cArray[0]);
	AssertInt(2, cArray[1]);
	AssertInt(3, cArray[2]);
	AssertInt(4, cArray[3]);
	AssertInt(5, cArray[4]);

	cArray.InsertIntoSorted(2, false);

	AssertSize(6, cArray.NumElements());
	AssertInt(1, cArray[0]);
	AssertInt(2, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(3, cArray[3]);
	AssertInt(4, cArray[4]);
	AssertInt(5, cArray[5]);

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntSortingUsingQuickSort(void)
{
	CArrayUint	cArray;

	cArray.Init();
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cArray.QuickSort();
	AssertSize(10, cArray.NumElements());
	AssertInt(0, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(3, cArray[4]);
	AssertInt(4, cArray[5]);
	AssertInt(4, cArray[6]);
	AssertInt(7, cArray[7]);
	AssertInt(8, cArray[8]);
	AssertInt(9, cArray[9]);

	cArray.InsertIntoSorted(7, false);
	AssertSize(11, cArray.NumElements());
	AssertInt(0, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(3, cArray[4]);
	AssertInt(4, cArray[5]);
	AssertInt(4, cArray[6]);
	AssertInt(7, cArray[7]);
	AssertInt(7, cArray[8]);
	AssertInt(8, cArray[9]);
	AssertInt(9, cArray[10]);

	cArray.InsertIntoSorted(1, true);
	AssertSize(11, cArray.NumElements());
	AssertInt(0, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(3, cArray[4]);
	AssertInt(4, cArray[5]);
	AssertInt(4, cArray[6]);
	AssertInt(7, cArray[7]);
	AssertInt(7, cArray[8]);
	AssertInt(8, cArray[9]);
	AssertInt(9, cArray[10]);

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntSortingUsingBubbleSort(void)
{
	CArrayUint	cArray;

	cArray.Init();
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cArray.BubbleSort();

	AssertSize(10, cArray.NumElements());
	AssertInt(0, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(3, cArray[4]);
	AssertInt(4, cArray[5]);
	AssertInt(4, cArray[6]);
	AssertInt(7, cArray[7]);
	AssertInt(8, cArray[8]);
	AssertInt(9, cArray[9]);

	cArray.InsertIntoSorted(7, false);
	cArray.InsertIntoSorted(1, true);

	AssertSize(11, cArray.NumElements());
	AssertInt(0, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(2, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(3, cArray[4]);
	AssertInt(4, cArray[5]);
	AssertInt(4, cArray[6]);
	AssertInt(7, cArray[7]);
	AssertInt(7, cArray[8]);
	AssertInt(8, cArray[9]);
	AssertInt(9, cArray[10]);

	cArray.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntRemoveDuplicates(void)
{
	CArrayUint	cArray;

	cArray.Init();
	cArray.AddList(-1, 1, 1, 2, 3, 3, 3, 4, 5, 5, -1);
	cArray.RemoveDuplicatesFromSorted();
	AssertSize(5, cArray.NumElements());
	AssertInt(1, cArray[0]);
	AssertInt(2, cArray[1]);
	AssertInt(3, cArray[2]);
	AssertInt(4, cArray[3]);
	AssertInt(5, cArray[4]);

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RemoveAtKiller(CArrayUint* pcArray, CArrayUint* pcKiller)
{
	CArrayTemplatePrimitive<size>	cIndices;
	size							i;

	//RemoveAt takes size indices.
	cIndices.Init();
	for (i = 0; i < pcKiller->NumElements(); i++)
	{
		cIndices.Add(pcKiller->GetValue(i));
	}
	pcArray->RemoveAt(cIndices.GetData(), cIndices.NumElements());
	cIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntRemoveAt(void)
{
	CArrayUint	cArray;
	CArrayUint	cKiller;

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	AssertSize(10, cArray.NumElements());

	cKiller.Init();
	cKiller.AddList(-1, 8, 5, 0, 5, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(7, cArray.NumElements());

	AssertInt(3, cArray[0]);
	AssertInt(7, cArray[1]);
	AssertInt(8, cArray[2]);
	AssertInt(1, cArray[3]);
	AssertInt(0, cArray[4]);
	AssertInt(2, cArray[5]);
	AssertInt(2, cArray[6]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	AssertSize(10, cArray.NumElements());

	cKiller.Init();
	cKiller.AddList(-1, 8, 5, 2, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(7, cArray.NumElements());

	AssertInt(4, cArray[0]);
	AssertInt(3, cArray[1]);
	AssertInt(8, cArray[2]);
	AssertInt(1, cArray[3]);
	AssertInt(0, cArray[4]);
	AssertInt(2, cArray[5]);
	AssertInt(2, cArray[6]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	AssertSize(10, cArray.NumElements());

	cKiller.Init();
	cKiller.AddList(-1, 0, 1, 2, 7, 8, 9, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(4, cArray.NumElements());

	AssertInt(8, cArray[0]);
	AssertInt(1, cArray[1]);
	AssertInt(9, cArray[2]);
	AssertInt(0, cArray[3]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	AssertSize(10, cArray.NumElements());

	cKiller.Init();
	cKiller.AddList(-1, 3, 4, 5, 6, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(6, cArray.NumElements());

	AssertInt(4, cArray[0]);
	AssertInt(3, cArray[1]);
	AssertInt(7, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(4, cArray[4]);
	AssertInt(2, cArray[5]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cKiller.Init();
	cKiller.AddList(-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(0, cArray.NumElements());

	cKiller.Kill();
	cArray.Kill();


	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cKiller.Init();
	cKiller.AddList(-1, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(10, cArray.NumElements());

	AssertInt(4, cArray[0]);
	AssertInt(3, cArray[1]);
	AssertInt(7, cArray[2]);
	AssertInt(8, cArray[3]);
	AssertInt(1, cArray[4]);
	AssertInt(9, cArray[5]);
	AssertInt(0, cArray[6]);
	AssertInt(2, cArray[7]);
	AssertInt(4, cArray[8]);
	AssertInt(2, cArray[9]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cKiller.Init();
	cKiller.AddList(-1, 0, 2, 4, 6, 8, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(5, cArray.NumElements());

	AssertInt(3, cArray[0]);
	AssertInt(8, cArray[1]);
	AssertInt(9, cArray[2]);
	AssertInt(2, cArray[3]);
	AssertInt(2, cArray[4]);

	cKiller.Kill();
	cArray.Kill();

	////////////////////////////////////////////////
	cArray.Init();

	//                 0  1  2  3  4  5  6  7  8  9  Stop
	cArray.AddList(-1, 4, 3, 7, 8, 1, 9, 0, 2, 4, 2, -1);

	cKiller.Init();
	cKiller.AddList(-1, 1, 3, 5, 7, 9, -1);
	cKiller.MakeUnique();

	RemoveAtKiller(&cArray, &cKiller);

	AssertSize(5, cArray.NumElements());

	AssertInt(4, cArray[0]);
	AssertInt(7, cArray[1]);
	AssertInt(1, cArray[2]);
	AssertInt(0, cArray[3]);
	AssertInt(4, cArray[4]);

	cKiller.Kill();
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntFindFinalContiguousInSorted(void)
{
	CArrayInt	cArray;

	cArray.Init();
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cArray.FindFinalContiguousInSorted());
	cArray.Add(0);
	AssertSize(0, cArray.FindFinalContiguousInSorted());
	cArray.Add(2);
	AssertSize(0, cArray.FindFinalContiguousInSorted());
	cArray.Kill();

	cArray.Init();
	cArray.Add(0);
	cArray.Add(1);
	cArray.Add(2);
	AssertSize(2, cArray.FindFinalContiguousInSorted());
	cArray.Add(4);
	AssertSize(2, cArray.FindFinalContiguousInSorted());
	cArray.Kill();

	cArray.Init();
	cArray.Add(1);
	cArray.Add(2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cArray.FindFinalContiguousInSorted());
	cArray.Kill();
}


//Orders by tens, so numbers in the same ten are equal.
struct STestArrayIntTensOrder
{
	bool	IsBefore(int iValue, int iOther);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool STestArrayIntTensOrder::IsBefore(int iValue, int iOther)
{
	return (iValue / 10) < (iOther / 10);
}


//////////////////////////////////////////////////////////////////////////
//
// Equal elements stay in the order they're inserted, and elements before
// iFirst aren't looked at.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayIntInsertInOrder(void)
{
	CArrayInt				cArray;
	STestArrayIntTensOrder	sOrder;

	cArray.Init();
	cArray.InsertInOrder(31, &sOrder);
	cArray.InsertInOrder(12, &sOrder);
	cArray.InsertInOrder(35, &sOrder);
	cArray.InsertInOrder(17, &sOrder);
	cArray.InsertInOrder(20, &sOrder);
	cArray.InsertInOrder(5, &sOrder);
	AssertSize(6, cArray.NumElements());
	AssertInt(5, cArray[0]);
	AssertInt(12, cArray[1]);
	AssertInt(17, cArray[2]);
	AssertInt(20, cArray[3]);
	AssertInt(31, cArray[4]);
	AssertInt(35, cArray[5]);

	cArray.InsertInOrder(1, &sOrder, 2);
	AssertSize(7, cArray.NumElements());
	AssertInt(12, cArray[1]);
	AssertInt(1, cArray[2]);
	AssertInt(17, cArray[3]);
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayInt(void)
{
	BeginTests();

	TestArrayIntAddFind();
	TestArrayIntSortingUsingQuickSort();
	TestArrayIntSortingUsingBubbleSort();
	TestArrayIntRemoveDuplicates();
	TestArrayIntRemoveAt();
	TestArrayIntFindFinalContiguousInSorted();
	TestArrayIntInsertInOrder();

	TestStatistics();
}

