#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerNegation(void)
{
	ObjectsInit();

	Ptr<CTestObject>	pcObject;
	CPointer			pcBase;

	pcObject = OMalloc<CTestObject>();
	AssertFalse(!pcObject);

	pcObject = NULL;
	AssertTrue(!pcObject);

	pcBase = OMalloc<CTestObject>();
	AssertFalse(!pcBase);

	pcBase = NULL;
	AssertTrue(!pcBase);

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerConstructor(void)
{
	ObjectsInit();

	CEmbeddedContainer	cCluster;

	cCluster.Init();
	AssertNull(cCluster.mpTest.Object());
	AssertPointer(&cCluster, cCluster.mpTest.Embedding());

	ObjectsKill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerDestructorDestruct(STestObjectFreedNotifier* psFreedNotifier)
{
	CTestObject					cTestObject;

	AssertLong(0, gcObjects.NumMemoryIndexes());

	cTestObject.Init(NULL);
	cTestObject.mpTest = OMalloc<CTestObject>(psFreedNotifier);

	AssertLong(1, gcObjects.NumMemoryIndexes());
	//cTestObject Destructor called here.  It causes the allocated object it points to to be destroyed too.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerDestructor(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sFreedNotifier;

	TestPointerDestructorDestruct(&sFreedNotifier);
	
	AssertTrue(sFreedNotifier.bFreed);
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerStackToEmbeddedDestruct(void)
{
	AssertLong(0, gcObjects.NumMemoryIndexes());
	Ptr<CEmbeddedContainer> pMissile = OMalloc<CEmbeddedContainer>();

	AssertLong(1, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerStackToEmbedded(void)
{
	ObjectsInit();

	TestPointerStackToEmbeddedDestruct();
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerAssignmentSimple()
{
	ObjectsInit();

	Ptr<CTestWithArray>	pcObject1 = ONMalloc<CTestWithArray>("Urgle-burgle", "Warning", 17);

	AssertNotNull(&pcObject1);
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pcObject1 = NULL;

	AssertNull(&pcObject1);
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerAssignmentStackToObjectPointer()
{
	ObjectsInit();

	Ptr<CTestInteger>		pInteger = OMalloc<CTestInteger>(6, 7, 6);
	Ptr<CTestNamedString>	pObject = ONMalloc<CTestNamedString>("Urgle-burgle", Null(), pInteger, "Embedded");

	AssertLong(2, gcObjects.GetStackPointers()->NumElements());

	Ptr<CTestInteger> ptr = pObject->mpAnother;

	AssertLong(3, gcObjects.GetStackPointers()->NumElements());

	ObjectsFlush();
	ObjectsKill(false);

	AssertTrue(pObject.IsNull());
	AssertTrue(pInteger.IsNull());
	AssertTrue(ptr.IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerAssignmentObjectToObjectPointer()
{
	ObjectsInit();

	Ptr<CTestInteger>		pInteger = OMalloc<CTestInteger>(6, 7, 6);
	Ptr<CTestNamedString>	pObject = ONMalloc<CTestNamedString>("Urgle-burgle", Null(), pInteger, "Embedded");

	AssertLong(2, gcObjects.GetStackPointers()->NumElements());

	Ptr<CTestInteger> ptr = pObject->mpAnother;

	AssertLong(3, gcObjects.GetStackPointers()->NumElements());

	ObjectsFlush();
	ObjectsKill(false);

	AssertTrue(pObject.IsNull());
	AssertTrue(pInteger.IsNull());
	AssertTrue(ptr.IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerHeapNotInGraphFreeStack(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	CTestObject*				pcTest1;
	CTestObject*				pcTest2;

	Ptr<CTestObject> pTest1 = OMalloc<CTestObject>(&sNotifier1);
	Ptr<CTestObject> pTest2 = OMalloc<CTestObject>(&sNotifier2);

	AssertSize(0, pTest1->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumPointerTos());
	AssertSize(0, pTest2->NumHeapFroms());
	AssertSize(1, pTest2->NumStackFroms());
	AssertSize(0, pTest2->NumPointerTos());
	AssertFalse(sNotifier1.bFreed);
	AssertFalse(sNotifier2.bFreed);

	pTest2->mpObject = pTest1;

	AssertSize(1, pTest1->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumPointerTos());
	AssertSize(0, pTest2->NumHeapFroms());
	AssertSize(1, pTest2->NumStackFroms());
	AssertSize(1, pTest2->NumPointerTos());

	pcTest2 = (CTestObject*)pTest2.Object();
	pcTest1 = (CTestObject*)pTest1.Object();
	pTest1 = NULL;  //pTest1 should not be killed here, even though it has no stack froms.  It still has a heap from.

	AssertFalse(sNotifier1.bFreed);
	AssertFalse(sNotifier2.bFreed);
	AssertSize(1, pcTest1->NumHeapFroms());
	AssertSize(0, pcTest1->NumStackFroms());
	AssertSize(0, pcTest1->NumPointerTos());
	AssertSize(0, pcTest2->NumHeapFroms());
	AssertSize(1, pcTest2->NumStackFroms());
	AssertSize(1, pcTest2->NumPointerTos());

	pTest2 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerCopy(void)
{
	ObjectsInit();
	{
		CTestObject	cTestObject1;
		cTestObject1.Init();
		CTestObject	cTestObject2;
		cTestObject2.Init();
		AssertPointer(&cTestObject1, cTestObject1.mpObject.Embedding());
		cTestObject1.mpObject = &cTestObject2;
		AssertPointer(&cTestObject1, cTestObject1.mpObject.Embedding());
		Ptr<CTestObject> pTest1 = cTestObject1.mpObject;
		AssertNull(pTest1.Embedding());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointer(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestPointerConstructor();
	TestPointerDestructor();
	TestPointerStackToEmbedded();
	TestPointerAssignmentSimple();
	TestPointerAssignmentStackToObjectPointer();
	TestPointerAssignmentObjectToObjectPointer();
	TestPointerNegation();
	TestPointerHeapNotInGraphFreeStack();
	TestPointerCopy();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

