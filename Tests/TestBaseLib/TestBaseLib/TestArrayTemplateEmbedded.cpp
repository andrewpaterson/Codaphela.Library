#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/ArrayTemplateEmbeddedPrimitive.h"
#include "TestLib/Assert.h"
#include "ArrayTemplateTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedAdd(void)
{
	CArrayTemplateEmbedded<int, 6>	cArray;
	int*							pi;
	int*							piEmbeddedData;

	cArray.Init();
	AssertSize(0, cArray.NumElements());
	piEmbeddedData = cArray.GetData();

	pi = cArray.Add();
	*pi = 7;
	AssertSize(1, cArray.NumElements());
	AssertPointer(piEmbeddedData, cArray.GetData());

	pi = cArray.Get(0);
	AssertInt(7,* pi);

	cArray.AddNum(4);
	AssertSize(5, cArray.NumElements());
	AssertPointer(piEmbeddedData, cArray.GetData());
	*cArray.Get(1) = 6;
	*cArray.Get(2) = 5;
	*cArray.Get(3) = 4;
	*cArray.Get(4) = 3;
	AssertTrue(cArray.IsEmbedded());
	AssertFalse(cArray.IsArray());

	pi = cArray.Add();
	*pi = 22;
	AssertSize(6, cArray.NumElements());
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertInt(7, *cArray.Get(0));
	AssertInt(22, *cArray.Get(5));
	AssertTrue(cArray.IsEmbedded());
	AssertFalse(cArray.IsArray());

	pi = cArray.Add();
	*pi = 20;
	AssertSize(7, cArray.NumElements());
	AssertFalse(piEmbeddedData == cArray.GetData());

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(5, *cArray.Get(2));
	AssertInt(4, *cArray.Get(3));
	AssertInt(3, *cArray.Get(4));
	AssertInt(22, *cArray.Get(5));
	AssertInt(20, *cArray.Get(6));

	AssertTrue(cArray.TestInternalConsistency());
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedRemoveAt(void)
{
	CArrayTemplateEmbedded<int, 5>	cArray;
	int*							piEmbeddedData;
	int*							pi;
	int								i;

	cArray.Init();
	piEmbeddedData = cArray.GetData();
	cArray.AddNum(6);
	AssertFalse(piEmbeddedData == cArray.GetData());
	AssertSize(6, cArray.NumElements());

	*cArray.Get(0) = 7;
	*cArray.Get(1) = 6;
	*cArray.Get(2) = 5;
	*cArray.Get(3) = 4;
	*cArray.Get(4) = 22;
	*cArray.Get(5) = 20;

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(5, *cArray.Get(2));
	AssertInt(4, *cArray.Get(3));
	AssertInt(22, *cArray.Get(4));
	AssertInt(20, *cArray.Get(5));
	
	cArray.Resize(5);
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertSize(5, cArray.NumElements());

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(5, *cArray.Get(2));
	AssertInt(4, *cArray.Get(3));
	AssertInt(22, *cArray.Get(4));

	pi = cArray.Add();
	*pi = 20;
	AssertSize(6, cArray.NumElements());
	AssertFalse(piEmbeddedData == cArray.GetData());

	cArray.RemoveAt(2);
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertSize(5, cArray.NumElements());

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(4, *cArray.Get(2));
	AssertInt(22, *cArray.Get(3));
	AssertInt(20, *cArray.Get(4));

	cArray.RemoveAt(3);
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertSize(4, cArray.NumElements());

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(4, *cArray.Get(2));
	AssertInt(20, *cArray.Get(3));

	i = 8;
	cArray.InsertAt(&i, 4);
	AssertSize(5, cArray.NumElements());
	AssertInt(8, *cArray.Get(4));

	i = 9;
	cArray.InsertAt(&i, 5);
	AssertFalse(piEmbeddedData == cArray.GetData());
	AssertSize(6, cArray.NumElements());
	AssertInt(8, *cArray.Get(4));
	AssertInt(9, *cArray.Get(5));

	cArray.RemoveAt(5);
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertSize(5, cArray.NumElements());

	AssertInt(7, *cArray.Get(0));
	AssertInt(6, *cArray.Get(1));
	AssertInt(4, *cArray.Get(2));
	AssertInt(20, *cArray.Get(3));
	AssertInt(8, *cArray.Get(4));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedRemove(void)
{
	CArrayTemplateEmbedded<int*, 2>	cArray;

	int								i1;
	int								i2;

	int*							pi1;
	int*							pi2;

	int**							ppi1;
	int**							ppi2;

	size							iIndex;

	i1 = 1;
	i2 = 2;

	pi1 = &i1;
	pi2 = &i2;

	cArray.Init();
	ppi1 = cArray.Add(&pi1);
	ppi2 = cArray.Add(&pi2);

	iIndex = cArray.GetIndex(&pi1);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	iIndex = cArray.GetIndex(ppi1);
	AssertSize(0, iIndex);
	iIndex = cArray.GetIndex(&pi2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	iIndex = cArray.GetIndex(ppi2);
	AssertSize(1, iIndex);

	iIndex = cArray.Find(&pi1);
	AssertSize(0, iIndex);
	iIndex = cArray.Find(ppi1);
	AssertSize(0, iIndex);
	iIndex = cArray.Find(&pi2);
	AssertSize(1, iIndex);
	iIndex = cArray.Find(ppi2);
	AssertSize(1, iIndex);

	cArray.Remove(&pi1);
	iIndex = cArray.Find(&pi1);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	AssertPointer(pi2, *cArray.Get(0));
}


typedef CArrayTemplateEmbedded<CTestOverridenArrayTemplateItem, 3> CTestArrayTemplateEmbedded;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedSizeof(void)
{
#ifdef BUILD_64
	AssertSize(48, 3 * sizeof(CTestOverridenArrayTemplateItem));
	AssertSize(48, sizeof(CArrayTemplate<CTestOverridenArrayTemplateItem>));

	//The int is padded to the alignment of the item's virtual function table pointer.
	AssertSize(sizeof(void*) + 3 * sizeof(CTestOverridenArrayTemplateItem), sizeof(CTestArrayTemplateEmbedded));
#elif defined(SIZE_16)
	AssertSize(36, 3 * sizeof(CTestOverridenArrayTemplateItem));
	AssertSize(16, sizeof(CArrayTemplate<CTestOverridenArrayTemplateItem>));

	AssertSize(sizeof(void*) + 3 * sizeof(CTestOverridenArrayTemplateItem), sizeof(CTestArrayTemplateEmbedded));
#else
	AssertSize(36, 3 * sizeof(CTestOverridenArrayTemplateItem));
	AssertSize(24, sizeof(CArrayTemplate<CTestOverridenArrayTemplateItem>));

	AssertSize(sizeof(void*) + 3 * sizeof(CTestOverridenArrayTemplateItem), sizeof(CTestArrayTemplateEmbedded));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedAllocationWithVirtualClass(void)
{
	CTestArrayTemplateEmbedded			ac;
	CTestOverridenArrayTemplateItem*	pc;

	ac.Init();

	pc = ac.Add();
	pc->Init(5);
	AssertInt((int32)5, pc->i1);
	AssertInt((int32)33, pc->i2);

	pc = ac.InsertAt(0);
	pc->Init(6);
	AssertInt((int32)6, pc->i1);
	AssertInt((int32)33, pc->i2);

	pc = ac.Add();
	pc->Init(7);
	AssertInt((int32)7, pc->i1);
	AssertInt((int32)33, pc->i2);
	AssertPointer(ac.Get(2), pc);

	pc = ac.Add();
	pc->Init(8);
	AssertInt((int32)8, pc->i1);
	AssertInt((int32)33, pc->i2);
	AssertPointer(ac.Get(3), pc);

	AssertSize(4, ac.NumElements());

	pc = ac.InsertAt(3);
	pc->Init(9);
	AssertInt((int32)9, pc->i1);
	AssertInt((int32)33, pc->i2);
	AssertPointer(ac.Get(3), pc);
	pc = ac.Get(4);
	AssertInt((int32)8, pc->i1);
	AssertInt((int32)33, pc->i2);

	ac.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedChangeChunkSize(void)
{
	CArrayTemplateEmbedded<int, 3>	cArray;
	int*							pi;
	int*							piEmbeddedData;
	size							i;

	cArray.Init();
	AssertSize(0, cArray.NumElements());
	piEmbeddedData = cArray.GetData();

	for (i = 0; i < 3; i++)
	{
		pi = cArray.Add();
		*pi = (int)i + 1;
	}
	AssertSize(3, cArray.NumElements());
	AssertPointer(piEmbeddedData, cArray.GetData());
	AssertTrue(cArray.IsEmbedded());
	AssertFalse(cArray.IsArray());

	pi = cArray.Add();
	*pi = (int)i + 1;
	AssertFalse(cArray.IsEmbedded());
	AssertTrue(cArray.IsArray());

	cArray.RemoveAt(0);
	AssertTrue(cArray.IsEmbedded());
	AssertFalse(cArray.IsArray());
	AssertSize(3, cArray.NumElements());
	AssertInt(2, *cArray.Get(0));
	AssertInt(3, *cArray.Get(1));
	AssertInt(4, *cArray.Get(2));

	for (i = 5; i < 30; i++)
	{
		pi = cArray.Add();
		*pi = (int)i;
	}
	AssertSize(28, cArray.NumElements());
	AssertFalse(cArray.IsEmbedded());
	AssertTrue(cArray.IsArray());

	for (i = 0; i < 28; i++)
	{
		AssertInt((int)i + 2, *cArray.Get(i)); 
	}

	for (i = 0; i < 26; i++)
	{
		cArray.RemoveAt(1);
	}
	AssertTrue(cArray.IsEmbedded());
	AssertFalse(cArray.IsArray());
	AssertInt(2, *cArray.Get(0));
	AssertInt(29, *cArray.Get(1));

	AssertTrue(cArray.TestInternalConsistency());
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbeddedPrimitiveSorted(void)
{
	CArrayTemplateEmbeddedPrimitive<int, 4>	cArray;
	CArrayTemplateEmbeddedPrimitive<int, 4>	cOther;
	CArrayTemplateEmbeddedPrimitive<int, 4>	cBoth;

	cArray.Init();
	cArray.InsertIntoSorted(5, false);
	cArray.InsertIntoSorted(1, false);
	cArray.InsertIntoSorted(4, false);
	AssertTrue(cArray.IsEmbedded());
	cArray.InsertIntoSorted(2, false);
	cArray.InsertIntoSorted(3, false);
	cArray.InsertIntoSorted(6, false);
	AssertTrue(cArray.IsArray());
	AssertSize(6, cArray.NumElements());
	AssertTrue(cArray.IsSorted());
	AssertInt(1, cArray.GetValue(0));
	AssertInt(3, cArray.GetValue(2));
	AssertInt(6, cArray.GetValue(5));

	AssertTrue(cArray.RemoveFromSorted(4));
	AssertFalse(cArray.RemoveFromSorted(7));
	AssertSize(5, cArray.NumElements());
	AssertInt(5, cArray.GetValue(3));

	cArray.InsertIntoSorted(3, false);
	AssertSize(6, cArray.NumElements());
	AssertTrue(cArray.RemoveDuplicatesFromSorted());
	AssertFalse(cArray.RemoveDuplicatesFromSorted());
	AssertSize(5, cArray.NumElements());
	AssertInt(3, cArray.GetValue(2));
	AssertInt(5, cArray.GetValue(3));
	cArray.Kill();

	cArray.Init();
	cArray.Add(9);
	cArray.Add(3);
	cArray.Add(7);
	cArray.BubbleSort();
	AssertTrue(cArray.IsEmbedded());
	AssertInt(3, cArray.GetValue(0));
	AssertInt(9, cArray.GetValue(2));
	cArray.Add(1);
	cArray.Add(8);
	cArray.Add(3);
	cArray.MakeUnique();
	AssertTrue(cArray.IsSorted());
	AssertSize(5, cArray.NumElements());
	AssertInt(1, cArray.GetValue(0));
	AssertInt(9, cArray.GetValue(4));

	cOther.Init();
	cOther.Add(8);
	cOther.Add(2);
	cOther.Add(1);
	cOther.QuickSort();
	AssertInt(1, cOther.GetValue(0));
	AssertInt(8, cOther.GetValue(2));
	cBoth.Init();
	cBoth.Intersect(&cArray, &cOther);
	AssertSize(2, cBoth.NumElements());
	AssertInt(1, cBoth.GetValue(0));
	AssertInt(8, cBoth.GetValue(1));
	cBoth.Kill();
	cOther.Kill();
	cArray.Kill();

	cArray.Init();
	cArray.Add(0);
	cArray.Add(1);
	cArray.Add(2);
	cArray.Add(3);
	cArray.Add(5);
	AssertSize(3, cArray.FindFinalContiguousInSorted());
	cArray.Kill();

	cArray.Init();
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cArray.FindFinalContiguousInSorted());
	cArray.Add(0);
	AssertSize(0, cArray.FindFinalContiguousInSorted());
	cArray.Add(2);
	AssertSize(0, cArray.FindFinalContiguousInSorted());
	cArray.Kill();

	cArray.Init();
	cArray.Add(1);
	cArray.Add(2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cArray.FindFinalContiguousInSorted());
	cArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayTemplateEmbedded(void)
{
	BeginTests();

	TestArrayTemplateEmbeddedAdd();
	TestArrayTemplateEmbeddedRemoveAt();
	TestArrayTemplateEmbeddedRemove();
	TestArrayTemplateEmbeddedSizeof();
	TestArrayTemplateEmbeddedAllocationWithVirtualClass();
	TestArrayTemplateEmbeddedChangeChunkSize();
	TestArrayTemplateEmbeddedPrimitiveSorted();

	TestStatistics();
}

