#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/ArrayCommonObject.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayCommonObjectPointerRemapping(void)
{
	Ptr<CArrayCommonObject>		pArray;
	Ptr<CRoot>					pRoot;
	Ptr<CTestObject>			pObject1;
	Ptr<CTestSaveableObject1>	pObject2;
	Ptr<CTestSaveableObject1>	pObject3;
	STestObjectFreedNotifier	sFreedNotifier;
	CPointer					pNull;
	size						uiNumRemapped;
	
	ObjectsInit();

	pRoot = ORoot();
	pArray = OMalloc<CArrayCommonObject>(false, false, true, false);
	pRoot->Add(pArray);

	pObject1 = OMalloc<CTestObject>(&sFreedNotifier);
	pObject2 = ONMalloc<CTestSaveableObject1>("Fred");
	pObject3 = ONMalloc<CTestSaveableObject1>("Barney");
	pNull = NULL;

	pArray->Add(pNull);
	pArray->Add(pObject2);
	pArray->Add(pObject1);
	pArray->Add(pObject2);
	pArray->Add(pObject1);
	pArray->Add(pObject2);
	AssertSize(6, pArray->NumElements());
	AssertSize(5, pArray->NonNullElements());

	sFreedNotifier.bFreed = false;

	AssertSize(5, pArray->NumPointerTos());
	AssertSize(1, pArray->NumHeapFroms());
	AssertSize(2, pObject1->NumHeapFroms());
	AssertSize(0, pObject1->NumPointerTos());
	AssertSize(3, pObject2->NumHeapFroms());
	AssertSize(0, pObject2->NumPointerTos());
	AssertSize(0, pObject3->NumHeapFroms());
	AssertSize(0, pObject3->NumPointerTos());

	uiNumRemapped = pObject1.MorphInto(&pObject3);
	AssertSize(3, uiNumRemapped);

	AssertSize(6, pArray->NumElements());
	AssertSize(5, pArray->NonNullElements());
	AssertNull(pArray->UnsafeGetObject(0));
	AssertPointer(&pObject2, pArray->UnsafeGetObject(1));
	AssertPointer(&pObject3, pArray->UnsafeGetObject(2));
	AssertPointer(&pObject2, pArray->UnsafeGetObject(3));
	AssertPointer(&pObject3, pArray->UnsafeGetObject(4));
	AssertPointer(&pObject2, pArray->UnsafeGetObject(5));

	AssertSize(5, pArray->NumPointerTos());
	AssertSize(1, pArray->NumHeapFroms());
	AssertTrue(sFreedNotifier.bFreed);
	AssertSize(3, pObject2->NumHeapFroms());
	AssertSize(0, pObject2->NumPointerTos());
	AssertSize(2, pObject3->NumHeapFroms());
	AssertSize(0, pObject3->NumPointerTos());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayCommonObjectMorphInto(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;
	STestObjectFreedNotifier	sNotifier4;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArrayObject>			pArray1;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestTriPointerObject>	pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestTriPointerObject>	pTest4;
		bool						bResult;
		size						uiRemapped;

		pRoot = ORoot();
		pArray1 = OMalloc<CArrayObject>();
		pRoot->Add(pArray1);

		pTest1 = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestTriPointerObject>(&sNotifier2);
		pTest3 = OMalloc<CTestObject>(&sNotifier3);
		pTest4 = OMalloc<CTestTriPointerObject>(&sNotifier4);
		bResult = pArray1->Add(pTest1);
		AssertTrue(bResult);
		bResult = pArray1->Add(pTest2);

		CArrayObject		cArray2;

		cArray2.Init();
		cArray2.Add(pTest2);
		cArray2.Add(pTest3);
		cArray2.Add(pTest2);

		pTest4->mpObject1 = pTest2;
		pTest4->mpObject2 = pTest2;

		pTest2->mpObject1 = pTest1;

		Ptr<CTestObjectWithFields>	pTest5;
		CPointer					pPtr;

		pTest5 = OMalloc<CTestObjectWithFields>();

		AssertFalse(sNotifier2.bFreed);
		uiRemapped = pTest2.MorphInto(&pTest5);
		//2 (cArray2) +
		//1 (pArray1) +
		//1 (pTest2)  +
		//2 (pTest4->mpObject1, pTest4->mpObject2) =
		//6
		AssertSize(6, uiRemapped);
		AssertPointer(&pTest5, &pTest2);
		AssertString("CTestObjectWithFields", pTest5->ClassName());
		AssertTrue(sNotifier2.bFreed);
		AssertPointer(&pTest5, &pTest4->mpObject1);
		AssertPointer(&pTest5, &pTest4->mpObject2);
		pPtr = pArray1->Get(0);
		AssertPointer(&pTest1, &pPtr);
		pPtr = pArray1->Get(1);
		AssertPointer(&pTest5, &pPtr);
		pPtr = cArray2.Get(0);
		AssertPointer(&pTest5, &pPtr);
		pPtr = cArray2.Get(1);
		AssertPointer(&pTest3, &pPtr);
		pPtr = cArray2.Get(2);
		AssertPointer(&pTest5, &pPtr);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayCommonObjectInsertNull(void)
{
	Ptr<CArrayObject>			pArray;
	Ptr<CRoot>					pRoot;
	Ptr<CTestSaveableObject1>	pObject1;
	Ptr<CTestSaveableObject1>	pObject2;
	CPointer					pNull;

	ObjectsInit();

	pRoot = ORoot();
	pArray = OMalloc<CArrayObject>();
	pRoot->Add(pArray);

	pObject1 = ONMalloc<CTestSaveableObject1>("Fred");
	pObject2 = ONMalloc<CTestSaveableObject1>("Barney");
	pNull = NULL;

	pArray->Add(pObject1);
	pArray->Add(pNull);
	AssertSize(2, pArray->NumElements());
	AssertSize(1, pArray->NonNullElements());

	AssertTrue(pArray->Insert(0, pNull));
	AssertSize(3, pArray->NumElements());
	AssertSize(1, pArray->NonNullElements());

	AssertTrue(pArray->Insert(2, pObject2));
	AssertSize(4, pArray->NumElements());
	AssertSize(2, pArray->NonNullElements());

	AssertNull(pArray->UnsafeGetObject(0));
	AssertPointer(&pObject1, pArray->UnsafeGetObject(1));
	AssertPointer(&pObject2, pArray->UnsafeGetObject(2));
	AssertNull(pArray->UnsafeGetObject(3));

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayCommonObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestArrayCommonObjectPointerRemapping();
	TestArrayCommonObjectMorphInto();
	TestArrayCommonObjectInsertNull();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

