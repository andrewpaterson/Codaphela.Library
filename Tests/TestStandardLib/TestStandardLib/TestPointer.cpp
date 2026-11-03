#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"
#include "EmbeddedObjectTestClasses.h"
#include "ObjectTestSetup.h"


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
void TestPointerStackWalkList(void)
{
	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	STestObjectFreedNotifier	sNotifier3;

	ObjectsInit();
	{
		Ptr<CTestObject>	pTest;
		Ptr<CTestObject>	pTest2;
		Ptr<CTestObject>	pTest3;

		pTest = OMalloc<CTestObject>(&sNotifier1);
		pTest2 = OMalloc<CTestObject>(&sNotifier2);
		pTest3 = OMalloc<CTestObject>(&sNotifier3);
		pTest->mpTest = pTest2;
		pTest2->mpTest = pTest3;
		pTest2 = NULL;
		pTest3 = NULL;
		AssertLong(3, gcObjects.NumMemoryIndexes());

		//pTest is the only thing holding the list 1 -> 2 -> 3.  Walking it must free only the object walked off.
		pTest = pTest->mpTest;
		AssertTrue(sNotifier1.bFreed);
		AssertFalse(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertSize(1, pTest->NumStackFroms());
		AssertSize(0, pTest->NumHeapFroms());
		AssertInt(0x07070707, pTest->mi);

		pTest = pTest->mpTest;
		AssertTrue(sNotifier2.bFreed);
		AssertFalse(sNotifier3.bFreed);
		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertSize(1, pTest->NumStackFroms());
		AssertSize(0, pTest->NumHeapFroms());
		AssertInt(0x07070707, pTest->mi);

		pTest = pTest->mpTest;
		AssertTrue(sNotifier3.bFreed);
		AssertTrue(pTest.IsNull());
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerReturn(void)
{
	STestObjectFreedNotifier	sNotifier;
	CTestObject*				pcTest;

	ObjectsInit();
	{
		Ptr<CTestObject>	pTest;

		//Returning a NULL Pointer returns NULL.
		AssertNull(pTest.Return());
		AssertTrue(pTest.IsNull());

		//Returning an Object removes the Pointer's stack from without freeing the Object.
		pTest = OMalloc<CTestObject>(&sNotifier);
		pcTest = (CTestObject*)pTest.Return();
		AssertNotNull(pcTest);
		AssertTrue(pTest.IsNull());
		AssertFalse(sNotifier.bFreed);
		AssertSize(0, pcTest->NumStackFroms());
		AssertLong(1, gcObjects.NumMemoryIndexes());

		//Pointing to it again frees it when the Pointer is cleared.
		pTest = pcTest;
		pTest = NULL;
		AssertTrue(sNotifier.bFreed);
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// ValidatePtr checks with dynamic_cast: an object of the Pointer's class or of
// a class derived from it (the class can be abstract) is valid, as is NULL; an
// object of another class is not.  The class names need no objects made.
//
//////////////////////////////////////////////////////////////////////////
void TestPointerValidatePtr(void)
{
	SLogConfig	sLogConfig;

	ObjectsInit();
	{
		Ptr<CTestObject>		pTest;
		Ptr<CPlayerVehicle>		pVehicle;
		Ptr<CTestObject>		pWrong;
		CPointer				pSaveable;

		AssertTrue(pTest.IsInstanceOfTemplateClass());
		pTest = OMalloc<CTestObject>();
		AssertTrue(pTest.IsInstanceOfTemplateClass());

		pVehicle = OMalloc<CJeep>(Ptr<CGameWorld>());
		AssertTrue(pVehicle.IsInstanceOfTemplateClass());
		AssertString("CPlayerVehicle", pVehicle.TemplateClassName());
		AssertString("CJeep", pVehicle->ClassName());

		pSaveable = OMalloc<CTestSaveableObject1>();
		pWrong = pSaveable;
		AssertFalse(pWrong.IsInstanceOfTemplateClass());
		AssertString("CTestObject", pWrong.TemplateClassName());

		sLogConfig = gcLogger.SetSilent();
		ValidatePtr(pWrong);
		gcLogger.SetConfig(&sLogConfig);

		pTest = NULL;
		pVehicle = NULL;
		pWrong = NULL;
		pSaveable = NULL;
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerMovedLogsError(char* szErrors, size uiSize)
{
	CMemoryFile		cMemory;
	CFileBasic		cBasicMemory;
	SLogConfig		sLogConfig;

	memset(szErrors, 0, uiSize);
	sLogConfig = gcLogger.SetSilent();
	cMemory.Init();
	gcLogger.AddOutput(&cMemory);
	gcObjects.ValidateObjectsConsistency();
	gcLogger.RemoveOutput(&cMemory);
	gcLogger.SetConfig(&sLogConfig);

	cMemory.Open(EFM_Read);
	cBasicMemory.Init(&cMemory);
	if (cBasicMemory.Size() > 0)
	{
		cBasicMemory.ReadStringChars(szErrors, (int)((cBasicMemory.Size() < (filePos)uiSize - 1) ? cBasicMemory.Size() : (filePos)uiSize - 1));
	}
	cMemory.Close();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPointerMoved(void)
{
	STestObjectFreedNotifier	sNotifier;
	Ptr<CTestObject>*			pp;
	char						acHolder[sizeof(Ptr<CTestObject>)];
	char						acMoved[sizeof(Ptr<CTestObject>)];
	char						szErrors[1024];

	ObjectsInit();

	//A stack pointer (one that isn't a field of an object) in memory that doesn't move.
	pp = new (acHolder) Ptr<CTestObject>();
	*pp = OMalloc<CTestObject>(&sNotifier);
	TestPointerMovedLogsError(szErrors, sizeof(szErrors));
	AssertString("", szErrors);

	//Moved, as a CArrayTemplate moves its elements when it grows: the object still knows the pointer by its old address.
	memcpy(acMoved, acHolder, sizeof(acHolder));
	memset(acHolder, 0, sizeof(acHolder));
	TestPointerMovedLogsError(szErrors, sizeof(szErrors));
	AssertTrue(strstr(szErrors, "that no longer points to it.  Was the Pointer moved?") != NULL);

	//Put back so it can be released.
	memcpy(acHolder, acMoved, sizeof(acHolder));
	pp->~Ptr<CTestObject>();
	AssertTrue(sNotifier.bFreed);
	AssertLong(0, gcObjects.NumMemoryIndexes());

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
	TestPointerStackWalkList();
	TestPointerReturn();
	TestPointerMoved();
	TestPointerValidatePtr();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

