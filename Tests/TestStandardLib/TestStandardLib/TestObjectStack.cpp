#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "TestClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackInit(void)
{
	ObjectsInit();

	CTestObject					cObject;
	STestObjectFreedNotifier	sFreedNotifier;
	CClass*						pcClass;

	AssertFalse(cObject.IsAllocatedInObjects());
	AssertTrue(cObject.GetFlags() & OBJECT_FLAGS_CALLED_CONSTRUCTOR);
	AssertFalse(cObject.GetFlags() & OBJECT_FLAGS_CALLED_ALLOCATE);
	AssertFalse(cObject.GetFlags() & OBJECT_FLAGS_CALLED_INIT);
	AssertFalse(cObject.GetFlags() & OBJECT_FLAGS_CALLED_CLASS);
	AssertInt(0, cObject.GetDistToStack());
	AssertInt(UNATTACHED_DIST_TO_ROOT, cObject.GetDistToRoot());

	pcClass = cObject.GetClass();
	AssertNull(pcClass);

	cObject.Init(&sFreedNotifier);

	pcClass = cObject.GetClass();
	AssertNotNull(pcClass);


	AssertTrue(cObject.GetFlags() & OBJECT_FLAGS_CALLED_INIT);

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DestructorOutOfFrame(STestObjectFreedNotifier* psFreeNotifier)
{
	CTestObject					cObject;

	cObject.Init(psFreeNotifier);
	AssertTrue(cObject.GetFlags() & OBJECT_FLAGS_CALLED_INIT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackDestructor(void)
{
	STestObjectFreedNotifier	sFreedNotifier;

	ObjectsInit();

	DestructorOutOfFrame(&sFreedNotifier);
	AssertTrue(sFreedNotifier.bFreed);

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PointedToOutOfFrame(void)
{
	Ptr<CTestClass>		pHeapTestClass;
	CTestClass			cStackTestClass;

	cStackTestClass.Init();
	pHeapTestClass = OMalloc<CTestClass>();
	pHeapTestClass->mpObject = &cStackTestClass;
	pHeapTestClass->mDouble = 98457.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackPointedTo(void)
{
	ObjectsInit();

	PointedToOutOfFrame();

	ObjectsKill();
}

//////////////////////////////////////////////////////////////////////////
//
// The object is made in the caller's memory (as a local would be in this
// frame) and destroyed before returning, so the caller can see what the
// destructor did without reading a frame that has returned.
//
//////////////////////////////////////////////////////////////////////////
void ObjectReuseOutOfFrame(void* pvStorage)
{
	CTestClass*		pcStackTestClass;
	CChars			sz;

	pcStackTestClass = new(pvStorage) CTestClass;

	sz.Init(); 
	pcStackTestClass->PrintFlags(&sz);
	AssertString("CALLED_CONSTRUCTOR", sz.Text());
	sz.Kill();

	pcStackTestClass->Init();
	sz.Init();
	pcStackTestClass->PrintFlags(&sz);
	AssertString("CALLED_CONSTRUCTOR, DIRTY, CALLED_INIT, CALLED_CLASS", sz.Text());
	sz.Kill();

	pcStackTestClass->Kill();
	sz.Init();
	pcStackTestClass->PrintFlags(&sz);
	AssertString("CALLED_CONSTRUCTOR, DIRTY, CALLED_KILL, CALLED_CLASS", sz.Text());
	sz.Kill();


	pcStackTestClass->Init();
	sz.Init();
	pcStackTestClass->PrintFlags(&sz);
	AssertString("CALLED_CONSTRUCTOR, DIRTY, CALLED_INIT, CALLED_CLASS", sz.Text());
	sz.Kill();

	pcStackTestClass->Kill();
	sz.Init();
	pcStackTestClass->PrintFlags(&sz);
	AssertString("CALLED_CONSTRUCTOR, DIRTY, CALLED_KILL, CALLED_CLASS", sz.Text());
	sz.Kill();

	pcStackTestClass->~CTestClass();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackObjectReuse(void)
{
	int64			alliStorage[(sizeof(CTestClass) + sizeof(int64) - 1) / sizeof(int64)];  //int64s to align it.
	CTestClass*		pcTestClass;
	CChars			sz;

	ObjectsInit();

	ObjectReuseOutOfFrame(alliStorage);
	pcTestClass = (CTestClass*)alliStorage;
	sz.Init();
	pcTestClass->PrintFlags(&sz);
	AssertString("", sz.Text());
	sz.Kill();

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTestObject> CreateTestObject(void)
{
	Ptr<CTestObject>	pTestObject;

	pTestObject = OMalloc<CTestObject>();
	return pTestObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTestObject> CreateAndReturnTestObject(void)
{
	return CreateTestObject();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackAssignmentOperator(void)
{
	ObjectsInit();
	{
		CChars				sz;
		Ptr<CTestObject>	pTest;

		pTest = CreateTestObject();

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString(
"0: Pointer [CTestObject:1 Ru Su (CC, D, CA, CI, CC)]\n", 
			sz.Text());
		sz.Kill();

		pTest = NULL;

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("", sz.Text());
		sz.Kill();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStackAssignmentOperator2(void)
{
	ObjectsInit();
	{
		CChars				sz;
		Ptr<CTestObject>	pTest;

		pTest = CreateAndReturnTestObject();

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString(
"0: Pointer [CTestObject:1 Ru Su (CC, D, CA, CI, CC)]\n", 
			sz.Text());
		sz.Kill();

		pTest->Kill();

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("", sz.Text());
		sz.Kill();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectStack(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectStackAssignmentOperator();
	TestObjectStackAssignmentOperator2();
	TestObjectStackInit();
	TestObjectStackDestructor();
	TestObjectStackPointedTo();
	TestObjectStackObjectReuse();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

