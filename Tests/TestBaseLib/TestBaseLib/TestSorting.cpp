#include "BaseLib/ArrayInt.h"
#include "BaseLib/LinkedListTemplate.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArraySorting2(void)
{
	CArrayInt	cInts;

	cInts.Init();
	cInts.Add(4);
	cInts.Add(3);
	cInts.Add(2);
	cInts.Add(1);

	cInts.QuickSort();

	AssertInt(1, cInts.GetValue(0));
	AssertInt(2, cInts.GetValue(1));
	AssertInt(3, cInts.GetValue(2));
	AssertInt(4, cInts.GetValue(3));

	cInts.Kill();

	cInts.Init();
	cInts.Add(4);
	cInts.Add(3);
	cInts.Add(2);
	cInts.Add(1);

	cInts.BubbleSort();

	AssertInt(1, cInts.GetValue(0));
	AssertInt(2, cInts.GetValue(1));
	AssertInt(3, cInts.GetValue(2));
	AssertInt(4, cInts.GetValue(3));

	cInts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestListSorting(void)
{
	CLinkedListTemplate<int>	cInts;
	int*					pi;

	cInts.Init();
	pi = cInts.InsertAfterTail();
	*pi = 4;
	pi = cInts.InsertAfterTail();
	*pi = 3;
	pi = cInts.InsertAfterTail();
	*pi = 2;
	pi = cInts.InsertAfterTail();
	*pi = 1;

	cInts.BubbleSort(&ComparePrimitive<int>);

	pi = cInts.GetHead();
	AssertInt(1, *pi);
	pi = cInts.GetNext(pi);
	AssertInt(2, *pi);
	pi = cInts.GetNext(pi);
	AssertInt(3, *pi);
	pi = cInts.GetNext(pi);
	AssertInt(4, *pi);

	cInts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSorting(void)
{
	BeginTests();

	TestArraySorting2();
	TestListSorting();

	TestStatistics();
}

