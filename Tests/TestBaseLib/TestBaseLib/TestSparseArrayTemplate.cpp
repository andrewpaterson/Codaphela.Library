#include "TestLib/Assert.h"
#include "BaseLib/ArraySparseTemplate.h"


struct STestArraySparseTemplateItem
{
	int	i1;
};


typedef CArraySparseTemplate<STestArraySparseTemplateItem>	CTestSparseArray;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArraySparseTemplateInsertion(void)
{
	CTestSparseArray				cArray;
	STestArraySparseTemplateItem*	psItem;
	int								i;
	int								iFound;

	cArray.Init();

	psItem = cArray.Set(50);
	psItem->i1 = 500;
	psItem = cArray.Set(25);
	psItem->i1 = 250;
	psItem = cArray.Set(10);
	psItem->i1 = 100;

	AssertSize(1, cArray.TestGetRoot()->iLeafDepth);
	AssertSize(0, cArray.TestGetRoot()->psLeft->iLeafDepth);
	AssertSize(0, cArray.TestGetRoot()->psRight->iLeafDepth);

	psItem = cArray.Get(50);
	AssertInt(500, psItem->i1);
	psItem = cArray.Get(25);
	AssertInt(250, psItem->i1);
	psItem = cArray.Get(10);
	AssertInt(100, psItem->i1);

	psItem = cArray.Set(11);
	psItem->i1 = 110;
	psItem = cArray.Set(12);
	psItem->i1 = 120;
	psItem = cArray.Set(13);
	psItem->i1 = 130;
	psItem = cArray.Set(14);
	psItem->i1 = 140;
	psItem = cArray.Set(20);
	psItem->i1 = 200;
	psItem = cArray.Set(22);
	psItem->i1 = 220;

	psItem = cArray.Get(14);
	AssertInt(140, psItem->i1);
	psItem = cArray.Get(13);
	AssertInt(130, psItem->i1);
	psItem = cArray.Get(20);
	AssertInt(200, psItem->i1);
	psItem = cArray.Get(22);
	AssertInt(220, psItem->i1);

	psItem = cArray.Set(9);
	psItem->i1 = 90;

	AssertSize(3, cArray.TestGetRoot()->iLeafDepth);
	AssertSize(2, cArray.TestGetRoot()->psLeft->iLeafDepth);
	AssertSize(2, cArray.TestGetRoot()->psRight->iLeafDepth);

	psItem = cArray.Get(13);
	AssertInt(130, psItem->i1);


	for (i = 51; i < 70; i++)
	{
		psItem = cArray.Set(i);
		psItem->i1 = i*10;
	}
	iFound = 0;
	for (i = 0; i < 100; i++)
	{
		psItem = cArray.Get(i);
		if (psItem)
		{
			iFound ++;
		}
	}
	AssertInt(29, iFound);

	for (i = 0; i < 100; i++)
	{
		psItem = cArray.Set(i);
		psItem->i1 = i*10;
	}
	for (i = 0; i < 100; i++)
	{
		psItem = cArray.Get(i);
		AssertInt(i*10, psItem->i1);
	}

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArraySparseTemplateSetData(void)
{
	CTestSparseArray				cArray;
	STestArraySparseTemplateItem	sItem;
	STestArraySparseTemplateItem*	psItem;

	cArray.Init();
	AssertNull(cArray.Get(7));

	sItem.i1 = 70;
	psItem = cArray.Set(7, &sItem);
	AssertNotNull(psItem);
	AssertInt(70, psItem->i1);
	AssertPointer(psItem, cArray.Get(7));

	sItem.i1 = 30;
	cArray.Set(3, &sItem);
	sItem.i1 = 71;
	AssertPointer(psItem, cArray.Set(7, &sItem));
	AssertInt(71, cArray.Get(7)->i1);
	AssertInt(30, cArray.Get(3)->i1);
	AssertNull(cArray.Get(5));

	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArraySparseTemplate(void)
{
	BeginTests();

	TestArraySparseTemplateInsertion();
	TestArraySparseTemplateSetData();

	TestStatistics();
}

