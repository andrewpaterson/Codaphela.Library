#include "TestLib/Assert.h"
#include "BaseLib/ArrayDenseTemplate.h"


struct STestArrayDenseTemplateItem
{
	int32	i1;
};


typedef CArrayDenseTemplate<STestArrayDenseTemplateItem> CTestDenseArray;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayDenseTemplateInsertion(void)
{
	CTestDenseArray					cArray;
	STestArrayDenseTemplateItem*	psItem0;
	STestArrayDenseTemplateItem*	psItem1;
	STestArrayDenseTemplateItem*	psItem2;
	STestArrayDenseTemplateItem*	psItem3;
	STestArrayDenseTemplateItem*	psItem;
	int32							i;
	int32							iEnd;

	cArray.Init();

	psItem0 = cArray.Add();
	psItem0->i1 = 0;
	AssertPointer(psItem0, cArray.TestGetRoot()->GetLeft());

	psItem1 = cArray.Add();
	psItem1->i1 = 1;
	AssertPointer(psItem1, cArray.TestGetRoot()->GetRight());
	AssertSize(2, cArray.NumUsedElements());

	psItem2 = cArray.Add();
	psItem2->i1 = 2;

	psItem3 = cArray.Add();
	psItem3->i1 = 3;
	AssertSize(4, cArray.NumUsedElements());
	AssertSize(3, cArray.NumUsedNodes());

	for (i = 0; i < (int32)cArray.NumUsedElements(); i++)
	{
		psItem = cArray.Get(i);
		AssertInt(i, psItem->i1);
	}

	iEnd = (int32)cArray.NumUsedElements() + 100;
	for (i = (int32)cArray.NumUsedElements(); i < iEnd; i++)
	{
		psItem = cArray.Add();
		psItem->i1 = i;
	}
	for (i = 0; i < (int32)cArray.NumUsedElements(); i++)
	{
		psItem = cArray.Get(i);
		AssertInt(i, psItem->i1);
	}
	cArray.Kill();

	cArray.Init();
	psItem = cArray.Add();
	psItem->i1 = 0;

	psItem = cArray.InsertAt(0);
	psItem->i1 = -1;

	psItem = cArray.Get(0);
	AssertInt((int32)-1, psItem->i1);
	psItem = cArray.Get(1);
	AssertInt((int32)0, psItem->i1);

	iEnd = (int32)cArray.NumUsedElements()+100;
	for (i = (int32)cArray.NumUsedElements(); i < iEnd; i++)
	{
		psItem = cArray.InsertAt(0);
		psItem->i1 = -((int32)i);
	}
	for (i = 0; i < (int32)cArray.NumUsedElements(); i++)
	{
		psItem = cArray.Get(i);
		AssertInt(i-((int)cArray.NumUsedElements()-1), psItem->i1);
	}

	cArray.Kill();

	cArray.Init();
	psItem = cArray.Add();
	psItem->i1 = 0;

	psItem = cArray.InsertAt(1);
	psItem->i1 = 2;
	psItem = cArray.InsertAt(1);
	psItem->i1 = 1;
	psItem = cArray.InsertAt(3);
	psItem->i1 = 3;
	psItem = cArray.InsertAt(1);
	psItem->i1 = 100;
	psItem = cArray.InsertAt(0);
	psItem->i1 = 101;
	psItem = cArray.InsertAt(3);
	psItem->i1 = 102;
	psItem = cArray.InsertAt(6);
	psItem->i1 = 103;
	psItem = cArray.InsertAt(5);
	psItem->i1 = 104;

	AssertSize(9, cArray.NumUsedElements());

	psItem = cArray.Get(0);
	AssertInt((int32)101, psItem->i1);
	psItem = cArray.Get(1);
	AssertInt((int32)0, psItem->i1);
	psItem = cArray.Get(2);
	AssertInt((int32)100, psItem->i1);
	psItem = cArray.Get(3);
	AssertInt((int32)102, psItem->i1);
	psItem = cArray.Get(4);
	AssertInt((int32)1, psItem->i1);
	psItem = cArray.Get(5);
	AssertInt((int32)104, psItem->i1);
	psItem = cArray.Get(6);
	AssertInt((int32)2, psItem->i1);
	psItem = cArray.Get(7);
	AssertInt((int32)103, psItem->i1);
	psItem = cArray.Get(8);
	AssertInt((int32)3, psItem->i1);

	cArray.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayDenseTemplateRemoval(void)
{
	CTestDenseArray					cArray;
	STestArrayDenseTemplateItem*	psItem;
	size							i;
	bool							bResult;

	cArray.Init();

	for (i = 0; i < 10; i++)
	{
		psItem = cArray.Add();
		psItem->i1 = (int)i;

		bResult = cArray.TestStructure();
		AssertTrue(bResult);
		if (!bResult)
		{
			cArray.Kill();
			return;
		}
	}

	cArray.RemoveAt(5);
	psItem = cArray.Get(5);
	AssertInt((int32)6, psItem->i1);

	cArray.RemoveAt(2);
	psItem = cArray.Get(2);
	AssertInt((int32)3, psItem->i1);

	cArray.RemoveAt(6);
	psItem = cArray.Get(6);
	AssertInt((int32)9, psItem->i1);

	cArray.RemoveAt(0);
	psItem = cArray.Get(0);
	AssertInt((int32)1, psItem->i1);

	cArray.RemoveAt(1);
	psItem = cArray.Get(1);
	AssertInt((int32)4, psItem->i1);

	cArray.RemoveAt(2);
	psItem = cArray.Get(2);
	AssertInt((int32)7, psItem->i1);

	cArray.RemoveAt(2);
	psItem = cArray.Get(2);
	AssertInt((int32)9, psItem->i1);

	cArray.RemoveAt(0);
	psItem = cArray.Get(0);
	AssertInt((int32)4, psItem->i1);

	cArray.RemoveAt(1);
	psItem = cArray.Get(0);
	AssertInt((int32)4, psItem->i1);

	cArray.RemoveAt(0);
	AssertSize(0, cArray.NumUsedElements());

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayDenseTemplate(void)
{
	BeginTests();

	TestArrayDenseTemplateInsertion();
	TestArrayDenseTemplateRemoval();

	TestStatistics();
}

