#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "TestLib/Assert.h"
#include "ArrayTemplateTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedPtrAdd(void)
{
	CArrayTemplateEmbeddedPtr<int, 5>	cArray;
	int		ia, ib, ic;
	int*	pi;

	ia = 1;
	ib = 2;
	ic = 3;


	cArray.Init();
	cArray.Add(&ia);
	cArray.Add(&ib);
	cArray.Add(&ic);

	pi = cArray.GetPtr(0);
	AssertInt(1, *pi);
	pi = cArray.GetPtr(1);
	AssertInt(2, *pi);
	pi = cArray.GetPtr(2);
	AssertInt(3, *pi);

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedPtrQuickSort(void)
{
	CArrayTemplateEmbeddedPtr<int, 5>	cArray;
	int									ai[7];

	cArray.Init();
	cArray.Add(&ai[2]);
	cArray.Add(&ai[0]);
	cArray.Add(&ai[1]);
	AssertTrue(cArray.IsEmbedded());
	cArray.QuickSort();
	AssertPointer(&ai[0], cArray.GetPtr(0));
	AssertPointer(&ai[1], cArray.GetPtr(1));
	AssertPointer(&ai[2], cArray.GetPtr(2));

	cArray.Add(&ai[6]);
	cArray.Add(&ai[4]);
	cArray.Add(&ai[5]);
	cArray.Add(&ai[3]);
	AssertTrue(cArray.IsArray());
	cArray.QuickSort();
	AssertSize(7, cArray.NumElements());
	AssertPointer(&ai[0], cArray.GetPtr(0));
	AssertPointer(&ai[3], cArray.GetPtr(3));
	AssertPointer(&ai[6], cArray.GetPtr(6));
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedPtr(void)
{
	BeginTests();

	TestArrayTemplateEmbeddedPtrAdd();
	TestArrayTemplateEmbeddedPtrQuickSort();

	TestStatistics();
}

