#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/FileBasic.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "CollectionObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
// What is logged between StartCapturingLog and StopCapturingLog (up to
// uiSize - 1 characters) is put in szLogged instead of being output.
//
//////////////////////////////////////////////////////////////////////////
void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig)
{
	*psLogConfig = gcLogger.SetSilent();
	pcMemory->Init();
	gcLogger.AddOutput(pcMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize)
{
	CFileBasic	cBasicMemory;

	gcLogger.RemoveOutput(pcMemory);
	gcLogger.SetConfig(psLogConfig);

	memset(szLogged, 0, uiSize);
	pcMemory->Open(EFM_Read);
	cBasicMemory.Init(pcMemory);
	if (cBasicMemory.Size() > 0)
	{
		cBasicMemory.ReadStringChars(szLogged, (size)((cBasicMemory.Size() < (filePos)uiSize - 1) ? cBasicMemory.Size() : (filePos)uiSize - 1));
	}
	pcMemory->Close();
	cBasicMemory.Kill();
	pcMemory->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingOutOfOrderOnStack(void)
{
	STestObjectFreedNotifier	sFreeNotifierO1;
	STestObjectFreedNotifier	sFreeNotifierN1;
	STestObjectFreedNotifier	sFreeNotifierN2;
	STestObjectFreedNotifier	sFreeNotifierO2;

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject1;
		Ptr<CTestNesHolderObject>	pNesHolder1;
		Ptr<CTestNesHolderObject>	pNesHolder2;
		Ptr<CTestObject>			pObject2;
		CArrayEmbeddedObjectPtrE8	apNesObjects;
		CEmbeddedObject*			pcExpectedNesPointerToObject;
		

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject1 = OMalloc<CTestObject>(&sFreeNotifierO1);
		pNesHolder1 = OMalloc<CTestNesHolderObject>(&sFreeNotifierN1, pObject1);
		pNesHolder2 = OMalloc<CTestNesHolderObject>(&sFreeNotifierN2, (CPointer)NULL);
		pObject2 = OMalloc<CTestObject>(&sFreeNotifierO2);
		pNesHolder2->SetObject(pObject2);

		AssertSize(1, pObject1->NumHeapFroms());
		AssertSize(0, pNesHolder1->NumHeapFroms());
		AssertSize(0, pNesHolder2->NumHeapFroms());
		AssertSize(1, pObject2->NumHeapFroms());

		apNesObjects.Init();
		pObject1->GetNesPointerToThis(&apNesObjects);
		AssertSize(1, apNesObjects.NumElements());
		pcExpectedNesPointerToObject = apNesObjects.GetPtr(0);
		AssertPointer(pNesHolder1.Object(), pcExpectedNesPointerToObject);
		apNesObjects.Kill();

		apNesObjects.Init();
		pObject2->GetNesPointerToThis(&apNesObjects);
		AssertSize(1, apNesObjects.NumElements());
		pcExpectedNesPointerToObject = apNesObjects.GetPtr(0);
		AssertPointer(pNesHolder2.Object(), pcExpectedNesPointerToObject);
		apNesObjects.Kill();
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());
	ObjectsKill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingAllocate(bool bHolderFirst, Ptr<CTestObject>& pObject, STestObjectFreedNotifier* psFreeNotifierObject, STestObjectFreedNotifier* psFreeNotifierHolder)
{
	Ptr<CTestNesHolderObject>	pHolder;

	if (bHolderFirst)
	{
		pHolder = OMalloc<CTestNesHolderObject>(psFreeNotifierHolder, (CPointer)NULL);
		pObject = OMalloc<CTestObject>(psFreeNotifierObject);
		pHolder->SetObject(pObject);
	}
	else
	{
		pObject = OMalloc<CTestObject>(psFreeNotifierObject);
		pHolder = OMalloc<CTestNesHolderObject>(psFreeNotifierHolder, pObject);
	}

	//The holder is only reachable through the Object it depends on.
	pObject->mpObject = pHolder;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingScopeExit(bool bHolderFirst)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	STestObjectFreedNotifier	sFreeNotifierHolder;

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		TestObjectFreeOrderingAllocate(bHolderFirst, pObject, &sFreeNotifierObject, &sFreeNotifierHolder);
		AssertSize(2, gcObjects.NumMemoryIndexes());
		AssertFalse(sFreeNotifierObject.bFreed);
		AssertFalse(sFreeNotifierHolder.bFreed);
	}
	AssertTrue(sFreeNotifierObject.bFreed);
	AssertTrue(sFreeNotifierHolder.bFreed);
	AssertFalse(sFreeNotifierHolder.bDependsOnFreedFirst);
	//The holder is freed and deallocated in the first batch so the Object's pointer to it is cleared before the Object is freed.
	AssertTrue(sFreeNotifierObject.bObjectNullWhenFreed);
	AssertSize(0, gcObjects.NumMemoryIndexes());
	gcObjects.ValidateObjectsConsistency();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingExplicitKill(bool bHolderFirst)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	STestObjectFreedNotifier	sFreeNotifierHolder;

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		TestObjectFreeOrderingAllocate(bHolderFirst, pObject, &sFreeNotifierObject, &sFreeNotifierHolder);
		AssertSize(2, gcObjects.NumMemoryIndexes());

		pObject->Kill();
		AssertTrue(sFreeNotifierObject.bFreed);
		AssertTrue(sFreeNotifierHolder.bFreed);
		AssertFalse(sFreeNotifierHolder.bDependsOnFreedFirst);
		//The holder is freed and deallocated in the first batch so the Object's pointer to it is cleared before the Object is freed.
		AssertTrue(sFreeNotifierObject.bObjectNullWhenFreed);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingStackKill(void)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	STestObjectFreedNotifier	sFreeNotifierHolder;

	ObjectsInit();
	{
		CTestObject					cObject;
		Ptr<CTestNesHolderObject>	pHolder;

		cObject.Init(&sFreeNotifierObject);
		pHolder = OMalloc<CTestNesHolderObject>(&sFreeNotifierHolder, &cObject);

		//The holder is only reachable through the Object on the stack that it depends on.
		cObject.mpObject = pHolder;
		pHolder = NULL;
		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertFalse(sFreeNotifierHolder.bFreed);

		cObject.Kill();
		AssertTrue(sFreeNotifierObject.bFreed);
		AssertTrue(sFreeNotifierHolder.bFreed);
		AssertFalse(sFreeNotifierHolder.bDependsOnFreedFirst);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingExplicitKillTwoPointers(void)
{
	STestObjectFreedNotifier	sNotifierKilled;
	STestObjectFreedNotifier	sNotifierHolder;

	ObjectsInit();
	{
		Ptr<CTestObject>	pKilled;
		Ptr<CTestObject>	pHolder;

		pKilled = OMalloc<CTestObject>(&sNotifierKilled);
		pHolder = OMalloc<CTestObject>(&sNotifierHolder);

		//The holder has two pointers to the killed Object and is only reachable through it.
		pHolder->mpObject = pKilled;
		pHolder->mpTest = pKilled;
		pKilled->mpObject = pHolder;
		pHolder = NULL;
		AssertSize(2, pKilled->NumHeapFroms());

		pKilled->Kill();
		AssertTrue(sNotifierKilled.bFreed);
		AssertTrue(sNotifierHolder.bFreed);
		AssertFalse(sNotifierHolder.bObjectNullWhenFreed);
		AssertFalse(sNotifierHolder.bTestNullWhenFreed);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingExplicitKillEmbeddedPointer(void)
{
	STestObjectFreedNotifier	sNotifierKilled;
	STestObjectFreedNotifier	sNotifierEmbedded;

	ObjectsInit();
	{
		Ptr<CTestObject>				pKilled;
		Ptr<CTestEmbeddedTestObject>	pContainer;

		pKilled = OMalloc<CTestObject>(&sNotifierKilled);
		pContainer = OMalloc<CTestEmbeddedTestObject>(&sNotifierEmbedded);

		//An Object embedded in the container points to the killed Object and the container is only reachable through it.
		pContainer->mcEmbedded.mpObject = pKilled;
		pKilled->mpObject = pContainer;
		pContainer = NULL;

		pKilled->Kill();
		AssertTrue(sNotifierKilled.bFreed);
		AssertTrue(sNotifierEmbedded.bFreed);
		AssertFalse(sNotifierEmbedded.bObjectNullWhenFreed);
		AssertSize(0, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingExplicitKillSurvivor(void)
{
	STestObjectFreedNotifier	sNotifierKilled;
	STestObjectFreedNotifier	sNotifierSurvivor;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CTestObject>	pKilled;
		Ptr<CTestObject>	pSurvivor;

		pRoot = ORoot();
		pKilled = OMalloc<CTestObject>(&sNotifierKilled);
		pSurvivor = OMalloc<CTestObject>(&sNotifierSurvivor);
		pRoot->Add(pSurvivor);

		//The survivor is reachable from the Root so its pointer to the killed Object must be cleared, not put back.
		pSurvivor->mpTest = pKilled;
		AssertSize(1, pKilled->NumHeapFroms());

		pKilled->Kill();
		AssertTrue(sNotifierKilled.bFreed);
		AssertFalse(sNotifierSurvivor.bFreed);
		AssertTrue(pSurvivor->mpTest.IsNull());
		AssertSize(1, pSurvivor->NumHeapFroms());
		AssertSize(3, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();

		pSurvivor = NULL;
		AssertTrue(pRoot->RemoveAll());
		AssertTrue(sNotifierSurvivor.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingNestedRemoval(void)
{
	STestObjectFreedNotifier	sNotifierObject;
	STestObjectFreedNotifier	sNotifierHolder;
	STestObjectFreedNotifier	sNotifierReleased;

	ObjectsInit();
	{
		Ptr<CTestObject>					pObject;
		Ptr<CTestNesHolderReleaseObject>	pHolder;
		Ptr<CTestObject>					pReleased;

		//The holder depends on the Object so the Object is freed in the second batch.
		//When the holder is freed in the first batch it releases an Object that points to the Object.
		//Freeing the released Object finds the Object again and must not free it a second time.
		pObject = OMalloc<CTestObject>(&sNotifierObject);
		pHolder = OMalloc<CTestNesHolderReleaseObject>(&sNotifierHolder, pObject);
		pReleased = OMalloc<CTestObject>(&sNotifierReleased);
		pHolder->mpReleasedOnFree = pReleased;
		pReleased->mpTest = pObject;
		pObject->mpObject = pHolder;
		pHolder = NULL;
		pReleased = NULL;
		AssertSize(3, gcObjects.NumMemoryIndexes());
	}
	AssertTrue(sNotifierObject.bFreed);
	AssertTrue(sNotifierHolder.bFreed);
	AssertTrue(sNotifierReleased.bFreed);
	AssertFalse(sNotifierHolder.bDependsOnFreedFirst);
	AssertSize(0, gcObjects.NumMemoryIndexes());
	gcObjects.ValidateObjectsConsistency();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingCollection(bool bExplicitKill)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	size						uiNumChildrenWhenFreed;

	uiNumChildrenWhenFreed = 99;
	ObjectsInit();
	{
		Ptr<CTestObject>		pObject;
		Ptr<CTestArrayObject>	pHolder;

		pObject = OMalloc<CTestObject>(&sFreeNotifierObject);
		pHolder = OMalloc<CTestArrayObject>(0);
		pHolder->mpuiNumChildrenWhenFreed = &uiNumChildrenWhenFreed;
		pHolder->maChildren.Add(pObject);

		//The holder is only reachable through the Object in its array.
		pObject->mpObject = pHolder;
		pHolder = NULL;
		AssertSize(2, gcObjects.NumMemoryIndexes());

		if (bExplicitKill)
		{
			pObject->Kill();
		}
		else
		{
			pObject = NULL;
		}
		AssertTrue(sFreeNotifierObject.bFreed);
		AssertSize(0, gcObjects.NumMemoryIndexes());
	}

	if (bExplicitKill)
	{
		//KillInternal only puts back pointer fields from Objects killed with the killed Object, not collection entries.
		//So the holder's array has already lost the killed Object when the holder is freed.
		AssertSize(0, uiNumChildrenWhenFreed);
	}
	else
	{
		AssertSize(1, uiNumChildrenWhenFreed);
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingObjectsKill(bool bHolderFirst)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	STestObjectFreedNotifier	sFreeNotifierHolder;

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		//Rooted so both are still alive when Objects is killed.
		TestObjectFreeOrderingAllocate(bHolderFirst, pObject, &sFreeNotifierObject, &sFreeNotifierHolder);
		ORoot()->Add(pObject);
	}
	AssertFalse(sFreeNotifierObject.bFreed);
	AssertFalse(sFreeNotifierHolder.bFreed);

	ObjectsFlush();
	ObjectsKill();

	//The holder has a Nes pointer to the Object so it must be freed first, whatever the order they were allocated in.
	AssertTrue(sFreeNotifierObject.bFreed);
	AssertTrue(sFreeNotifierHolder.bFreed);
	AssertFalse(sFreeNotifierHolder.bDependsOnFreedFirst);
	//Nothing is deallocated until every Object has been freed so the Object still points to the holder.
	AssertFalse(sFreeNotifierObject.bObjectNullWhenFreed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingEvictInMemory(bool bHolderFirst)
{
	STestObjectFreedNotifier	sFreeNotifierObject;
	STestObjectFreedNotifier	sFreeNotifierHolder;

	ObjectsInit();
	{
		Ptr<CTestObject>	pObject;

		TestObjectFreeOrderingAllocate(bHolderFirst, pObject, &sFreeNotifierObject, &sFreeNotifierHolder);
		ORoot()->Add(pObject);
	}
	AssertFalse(sFreeNotifierObject.bFreed);
	AssertFalse(sFreeNotifierHolder.bFreed);

	gcObjects.EvictInMemory();

	AssertTrue(sFreeNotifierObject.bFreed);
	AssertTrue(sFreeNotifierHolder.bFreed);
	AssertFalse(sFreeNotifierHolder.bDependsOnFreedFirst);
	AssertFalse(sFreeNotifierObject.bObjectNullWhenFreed);
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingObjectsKillNestedRemoval(void)
{
	STestObjectFreedNotifier	sNotifierObject;
	STestObjectFreedNotifier	sNotifierHolder;
	STestObjectFreedNotifier	sNotifierReleased;

	ObjectsInit();
	{
		Ptr<CTestObject>					pObject;
		Ptr<CTestNesHolderReleaseObject>	pHolder;
		Ptr<CTestObject>					pReleased;

		//As TestObjectFreeOrderingNestedRemoval but everything is still rooted when Objects is killed.
		//Releasing an Object while Objects is being killed must not remove it a second time.
		pObject = OMalloc<CTestObject>(&sNotifierObject);
		pHolder = OMalloc<CTestNesHolderReleaseObject>(&sNotifierHolder, pObject);
		pReleased = OMalloc<CTestObject>(&sNotifierReleased);
		pHolder->mpReleasedOnFree = pReleased;
		pReleased->mpTest = pObject;
		pObject->mpObject = pHolder;
		ORoot()->Add(pObject);
	}
	AssertSize(5, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();

	AssertTrue(sNotifierObject.bFreed);
	AssertTrue(sNotifierHolder.bFreed);
	AssertTrue(sNotifierReleased.bFreed);
	AssertFalse(sNotifierHolder.bDependsOnFreedFirst);
}


//////////////////////////////////////////////////////////////////////////
//
// Objects freed together are freed in the order the walk finds them, not
// by address: allocated last to first, a chain cut from the root is still
// freed from its start.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingDiscoveryOrder(void)
{
	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;
	uint32						uiBefore;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CTestObject>	pA;
		Ptr<CTestObject>	pB;
		Ptr<CTestObject>	pC;

		pRoot = ORoot();
		pC = OMalloc<CTestObject>(&sNotifierC);
		pB = OMalloc<CTestObject>(&sNotifierB);
		pA = OMalloc<CTestObject>(&sNotifierA);
		pA->mpTest = pB;
		pB->mpTest = pC;
		pRoot->Add(pA);
		pB = NULL;
		pC = NULL;

		uiBefore = guiTestObjectsFreed;
		pRoot->Remove(pA);
		pA = NULL;
		AssertInt(uiBefore + 1, sNotifierA.uiFreedOrder);
		AssertInt(uiBefore + 2, sNotifierB.uiFreedOrder);
		AssertInt(uiBefore + 3, sNotifierC.uiFreedOrder);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A chain of Nes pointers (Holder needs B, which needs C) is freed holder
// first, even when C is found before B.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingNesChain(void)
{
	STestObjectFreedNotifier	sNotifierHolder;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLogged[1024];

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CArrayObject>			pArray;
		Ptr<CTestNesChainObject>	pHolder;
		Ptr<CTestNesChainObject>	pB;
		Ptr<CTestNesChainObject>	pC;

		//  Root -> Array -> C
		//             \---> Holder =Nes=> B =Nes=> C
		pRoot = ORoot();
		pArray = OMalloc<CArrayObject>();
		pHolder = OMalloc<CTestNesChainObject>(&sNotifierHolder);
		pB = OMalloc<CTestNesChainObject>(&sNotifierB);
		pC = OMalloc<CTestNesChainObject>(&sNotifierC);
		pHolder->mpNeeds = pB;
		pB->mpNeeds = pC;
		pArray->Add(pC);
		pArray->Add(pHolder);
		pRoot->Add(pArray);
		pHolder = NULL;
		pB = NULL;
		pC = NULL;

		StartCapturingLog(&cLog, &sLogConfig);
		pRoot->Remove(pArray);
		pArray = NULL;
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertString("", szLogged);
		AssertTrue(sNotifierHolder.bFreed);
		AssertTrue(sNotifierB.bFreed);
		AssertTrue(sNotifierC.bFreed);
		AssertTrue(sNotifierHolder.uiFreedOrder < sNotifierB.uiFreedOrder);
		AssertTrue(sNotifierB.uiFreedOrder < sNotifierC.uiFreedOrder);
		AssertFalse(sNotifierHolder.bDependsOnFreedFirst);
		AssertFalse(sNotifierB.bDependsOnFreedFirst);
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects that need each other (a Nes cycle) can't be ordered: it is an
// error, and they are freed anyway.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingNesCycle(void)
{
	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLogged[1024];

	ObjectsInit();
	{
		Ptr<CTestNesChainObject>	pA;
		Ptr<CTestNesChainObject>	pB;
		Ptr<CTestNesChainObject>	pC;

		//  A =Nes=> B =Nes=> C =Nes=> B
		pA = OMalloc<CTestNesChainObject>(&sNotifierA);
		pB = OMalloc<CTestNesChainObject>(&sNotifierB);
		pC = OMalloc<CTestNesChainObject>(&sNotifierC);
		pA->mpNeeds = pB;
		pB->mpNeeds = pC;
		pC->mpNeeds = pB;
		pB = NULL;
		pC = NULL;

		StartCapturingLog(&cLog, &sLogConfig);
		pA = NULL;
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertTrue(strstr(szLogged, "hold Nes pointers to each other in a cycle") != NULL);
		AssertTrue(sNotifierA.bFreed);
		AssertTrue(sNotifierB.bFreed);
		AssertTrue(sNotifierC.bFreed);
		AssertFalse(sNotifierA.bDependsOnFreedFirst);
		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object with a Nes pointer to itself isn't a cycle: it is freed after
// what needs it (here a chain, so the Nes pointed to objects are ordered),
// and nothing is logged.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrderingNesToItself(void)
{
	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLogged[1024];

	ObjectsInit();
	{
		Ptr<CTestNesChainObject>	pA;
		Ptr<CTestNesChainObject>	pB;
		Ptr<CTestNesChainObject>	pC;

		//  A =Nes=> B =Nes=> C =Nes=> C
		pA = OMalloc<CTestNesChainObject>(&sNotifierA);
		pB = OMalloc<CTestNesChainObject>(&sNotifierB);
		pC = OMalloc<CTestNesChainObject>(&sNotifierC);
		pA->mpNeeds = pB;
		pB->mpNeeds = pC;
		pC->mpNeeds = pC;
		pB = NULL;
		pC = NULL;

		StartCapturingLog(&cLog, &sLogConfig);
		pA = NULL;
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertString("", szLogged);
		AssertTrue(sNotifierA.uiFreedOrder < sNotifierB.uiFreedOrder);
		AssertTrue(sNotifierB.uiFreedOrder < sNotifierC.uiFreedOrder);
		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeOrdering(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectFreeOrderingOutOfOrderOnStack();
	TestObjectFreeOrderingScopeExit(false);
	TestObjectFreeOrderingScopeExit(true);
	TestObjectFreeOrderingExplicitKill(false);
	TestObjectFreeOrderingExplicitKill(true);
	TestObjectFreeOrderingCollection(false);
	TestObjectFreeOrderingCollection(true);
	TestObjectFreeOrderingStackKill();
	TestObjectFreeOrderingExplicitKillTwoPointers();
	TestObjectFreeOrderingExplicitKillEmbeddedPointer();
	TestObjectFreeOrderingExplicitKillSurvivor();
	TestObjectFreeOrderingNestedRemoval();
	TestObjectFreeOrderingObjectsKill(false);
	TestObjectFreeOrderingObjectsKill(true);
	TestObjectFreeOrderingEvictInMemory(false);
	TestObjectFreeOrderingEvictInMemory(true);
	TestObjectFreeOrderingObjectsKillNestedRemoval();
	TestObjectFreeOrderingDiscoveryOrder();
	TestObjectFreeOrderingNesChain();
	TestObjectFreeOrderingNesCycle();
	TestObjectFreeOrderingNesToItself();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

