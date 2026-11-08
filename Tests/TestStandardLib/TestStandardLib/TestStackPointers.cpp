#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/StackPointers.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Objects.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"
#include "StandardLib/MapObject.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersAdd(void)
{
	CStackPointers	cStackPointers;
	CPointer		cPointer1;
	CPointer		cPointer2;
	CStackPointer*	pcStackPointer11;
	CStackPointer*	pcStackPointer12;
	CStackPointer*	pcStackPointer13;
	CStackPointer*	pcStackPointer21;
	CStackPointer*	pcStackPointer22;

	cStackPointers.Init();

	pcStackPointer11 = cStackPointers.Add(cPointer1.This());
	AssertNotNull(pcStackPointer11);
	pcStackPointer11->LinkBefore(NULL);
	AssertNull(pcStackPointer11->GetNext());
	AssertSize(1, pcStackPointer11->NumPointers());

	pcStackPointer12 = cStackPointers.Add(cPointer1.This());
	AssertNotNull(pcStackPointer12);
	pcStackPointer12->LinkBefore(pcStackPointer11);
	AssertPointer(pcStackPointer11, pcStackPointer12->GetNext());
	AssertSize(2, pcStackPointer12->NumPointers());

	pcStackPointer13 = cStackPointers.Add(cPointer1.This());
	AssertNotNull(pcStackPointer13);
	pcStackPointer13->LinkBefore(pcStackPointer12);
	AssertPointer(pcStackPointer12, pcStackPointer13->GetNext());
	AssertPointer(pcStackPointer11, pcStackPointer12->GetNext());
	AssertNull(pcStackPointer11->GetNext());
	AssertPointer(pcStackPointer11, pcStackPointer13->FindLast());
	AssertSize(3, pcStackPointer13->NumPointers());

	//A list linked in front of a list: 21, 22, then 13, 12, 11.
	pcStackPointer21 = cStackPointers.Add(cPointer2.This());
	pcStackPointer22 = cStackPointers.Add(cPointer2.This());
	pcStackPointer22->LinkBefore(NULL);
	pcStackPointer21->LinkBefore(pcStackPointer22);
	AssertSize(2, pcStackPointer21->NumPointers());
	pcStackPointer21->LinkBefore(pcStackPointer13);
	AssertPointer(pcStackPointer22, pcStackPointer21->GetNext());
	AssertPointer(pcStackPointer13, pcStackPointer22->GetNext());
	AssertSize(5, pcStackPointer21->NumPointers());
	AssertSize(5, cStackPointers.NumElements());

	cStackPointers.RemoveAll(pcStackPointer21);
	AssertSize(0, cStackPointers.NumElements());

	cStackPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object's stack froms are newest first.  Stack pointers going in the
// reverse of the order they came (as they usually do) each come off the
// front; going in any other order works as well.  HasStackFroms is true
// for an embedded object when only its container is held from the stack.
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersNewestFirst(void)
{
	ObjectsInit();
	{
		Ptr<CTestObject>			pObject;
		Ptr<CTestObject>			apHolders[8];
		Ptr<CEmbeddedContainer>		pContainer;
		Ptr<CEmbeddedContainer>		pSecond;
		CStackPointer*				pcFirst;
		size						i;

		pObject = OMalloc<CTestObject>();
		for (i = 0; i < 8; i++)
		{
			apHolders[i] = pObject;
			pcFirst = pObject->GetFirstStackFrom();
			AssertPointer(apHolders[i].This(), pcFirst->Get()->u.pcPointer);
		}
		AssertSize(9, pObject->NumStackFroms());

		for (i = 8; i > 4; i--)
		{
			apHolders[i - 1] = NULL;
			AssertPointer(apHolders[i - 2].This(), pObject->GetFirstStackFrom()->Get()->u.pcPointer);
		}
		AssertSize(5, pObject->NumStackFroms());
		for (i = 0; i < 4; i++)
		{
			apHolders[i] = NULL;
		}
		AssertSize(1, pObject->NumStackFroms());
		AssertPointer(pObject.This(), pObject->GetFirstStackFrom()->Get()->u.pcPointer);
		AssertTrue(pObject->HasStackFroms());

		pContainer = OMalloc<CEmbeddedContainer>();
		AssertTrue(pContainer->mcOne.HasStackFroms());
		AssertTrue(pContainer->mcTwo.HasStackFroms());
		AssertSize(0, pContainer->mcOne.CEmbeddedObject::NumStackFroms());
		pSecond = pContainer;
		pContainer = NULL;
		AssertTrue(pSecond->mcTwo.HasStackFroms());
		pSecond = NULL;
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersHeapKill(void)
{
	ObjectsInit();

	Ptr<CTestObject>			pTest1;
	Ptr<CTestObject>			pTest2;
	Ptr<CTestObject>			pTest3;
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
	pTest2 = OMalloc<CTestObject>(&sFreedNotifier2);
	AssertLong(1LL, pTest1.GetIndex());
	AssertLong(2LL, pTest2.GetIndex());
	pTest3 = pTest2;

	pTest1->mpTest = pTest2;

	//
	//        O[2]
	//        / ^ ^
	//       /  .  .
	//      /   .   .
	//     /    .    .
	//  O[1]  pTest2 pTest3
	//    ^     ^     ^  
	//    |     .     .  
	//  pTest1  .     .  
	//    ^     .     .  
	//    .     .     . 
	//

	AssertSize(2, pTest2->NumStackFroms());
	AssertSize(1, pTest2->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pTest3 = NULL;

	//
	//       O[2]
	//       / ^ 
	//      /   . 
	//     /     . 
	//  O[1]   pTest2
	//    ^       ^      -
	//    .       .      .
	//  pTest1    .    pTest3
	//    ^       .      ^  
	//    .       .      .    
	//

	AssertFalse(sFreedNotifier1.bFreed);
	AssertFalse(sFreedNotifier2.bFreed);
	AssertSize(1, pTest2->NumStackFroms());
	AssertSize(1, pTest2->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pTest1 = NULL;

	//      O[2]
	//        ^      -      -
	//        .      .	    .
	//     pTest2  pTest2  pTest3
	//        ^      ^      ^  
	//        .      .      .  

	AssertTrue(sFreedNotifier1.bFreed);
	AssertFalse(sFreedNotifier2.bFreed);
	AssertSize(1, pTest2->NumStackFroms());
	AssertSize(0, pTest2->NumHeapFroms());
	AssertLong(1, gcObjects.NumMemoryIndexes());

	pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
	AssertLong(3LL, pTest1.GetIndex());
	AssertLong(2LL, pTest2.GetIndex());
	pTest1->mpTest = pTest2;

	//
	//      O[2]
	//       / ^ 
	//      /   . 
	//     /     . 
	//  O[3]   pTest2
	//    ^       ^      -
	//    .       .      .
	//  pTest1    .    pTest3
	//    ^       .      ^  
	//    .       .      .  
	//

	AssertFalse(sFreedNotifier1.bFreed);
	AssertFalse(sFreedNotifier2.bFreed);
	AssertSize(1, pTest2->NumStackFroms());
	AssertSize(1, pTest2->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pTest2 = NULL;

	//
	//   O[2]
	//    ^
	//    |  
	//   O[3]  
	//    ^        -      -
	//    |   	   .      .
	//  pTest1	 pTest2  pTest3
	//    ^   	   ^       ^  
	//    .   	   .       .  
	//

	AssertFalse(sFreedNotifier1.bFreed);
	AssertFalse(sFreedNotifier2.bFreed);
	AssertSize(0, pTest1->mpTest->NumStackFroms());
	AssertSize(1, pTest1->mpTest->NumHeapFroms());
	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pTest1 = NULL;

	//
	//    -       -       -
	//    .       .       .
	//  pTest1  pTest2  pTest3
	//    ^       ^       ^  
	//    .       .       .  
	//

	AssertTrue(sFreedNotifier1.bFreed);
	AssertTrue(sFreedNotifier2.bFreed);
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersHeapKillSingle(void)
{
	ObjectsInit();

	Ptr<CTestObject>			pTest1;
	STestObjectFreedNotifier	sFreedNotifier1;

	pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);

	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(1, gcObjects.NumMemoryIndexes());

	pTest1 = NULL;

	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersKillSingle(void)
{
	ObjectsInit();

	Ptr<CTestObject>			pTest1;
	STestObjectFreedNotifier	sFreedNotifier1;

	pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);

	AssertSize(1, pTest1->NumStackFroms());
	AssertSize(0, pTest1->NumHeapFroms());
	AssertLong(1, gcObjects.NumMemoryIndexes());

	pTest1->Kill();
	AssertTrue(pTest1.IsNull());
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersSorting1(void)
{
	//Stack distance rework: You have no idea what causes the stack distance to be set.

	ObjectsInit();
	{
		Ptr<CTestEmbeddedObjectWithFields>	pTest1;
		Ptr<CTestEmbeddedObjectWithFields>	pTest2;
		Ptr<CTestEmbeddedObjectWithFields>	pTest3;
		Ptr<CTestEmbeddedObjectWithFields>	pTest4;
		Ptr<CTestEmbeddedObjectWithFields>	pTest5;
		Ptr<CTestEmbeddedObjectWithFields>	pTest6;
		CTestEmbeddedObjectWithFields*		pcTest5;

		pTest1 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest2 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest3 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest4 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest5 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest6 = OMalloc<CTestEmbeddedObjectWithFields>();
		pcTest5 = &pTest5;


		pTest1->mpObjectA = pTest2;
		pTest2->mpObjectA = pTest3;
		pTest3->mpObjectA = pTest4;
		pTest3->mpObjectB = pTest4;
		pTest4->mpObjectA = pTest5;
		pTest5->mpObjectA = pTest6;
		pTest2 = NULL;
		pTest3 = NULL;
		pTest4 = NULL;
		pTest5 = NULL;
		pTest6 = NULL;
		AssertSize(6, gcObjects.NumMemoryIndexes());

		pcTest5->mpObjectA = NULL;
		AssertSize(5, gcObjects.NumMemoryIndexes());

	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersRemoveMissing(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifierOther;

	ObjectsInit();
	{
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pOther;
		EFreeResult					eResult;
		bool						bResult;
		SLogConfig					sLogConfig;

		pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
		pTest2 = pTest1;
		pOther = OMalloc<CTestObject>(&sFreedNotifierOther);
		AssertSize(2, pTest1->NumStackFroms());
		AssertSize(3, gcStackPointers.NumElements());

		//pOther does not point to pTest1 so removing it must fail and leave pTest1's stack froms alone.
		sLogConfig = gcLogger.SetSilent();
		eResult = pTest1->RemoveStackFromTryFree(pOther.This());
		bResult = pTest1->TestRemoveStackFrom(pOther.This());
		gcLogger.SetConfig(&sLogConfig);

		AssertTrue(eResult == FR_Error);
		AssertFalse(bResult);
		AssertSize(2, pTest1->NumStackFroms());
		AssertSize(3, gcStackPointers.NumElements());
		AssertFalse(sFreedNotifier1.bFreed);
		AssertLong(2, gcObjects.NumMemoryIndexes());

		pTest2 = NULL;
		AssertSize(1, pTest1->NumStackFroms());
		AssertSize(2, gcStackPointers.NumElements());
		AssertFalse(sFreedNotifier1.bFreed);
	}
	AssertSize(0, gcStackPointers.NumElements());
	AssertLong(0, gcObjects.NumMemoryIndexes());

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// FromRemoved, once a from is unlinked, frees what nothing reaches and
// says so, and keeps what is still held.  An object on the stack is never
// freed by it: it is killed when it goes out of scope.
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersFromRemoved(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifierStack;

	ObjectsInit();
	{
		Ptr<CTestObject>	pTest1;
		Ptr<CTestObject>	pTest2;
		Ptr<CTestObject>	pStack;
		CTestObject			cStack;

		pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
		pTest2 = pTest1;

		//Still held by pTest2.
		AssertTrue(pTest1->UnlinkStackFrom(pTest1.This()));
		pTest1.UnsafeClearObject();
		AssertTrue(FR_NotFreed == gcObjects.FromRemoved(pTest2.Object(), false));
		AssertFalse(sFreedNotifier1.bFreed);

		//Nothing holds it.
		AssertTrue(pTest2->UnlinkStackFrom(pTest2.This()));
		AssertTrue(FR_Freed == gcObjects.FromRemoved(pTest2.Object(), false));
		pTest2.UnsafeClearObject();
		AssertTrue(sFreedNotifier1.bFreed);
		AssertLong(0, gcObjects.NumMemoryIndexes());

		cStack.Init(&sFreedNotifierStack);
		pStack = &cStack;
		AssertTrue(cStack.UnlinkStackFrom(pStack.This()));
		pStack.UnsafeClearObject();
		AssertTrue(FR_NotFreed == gcObjects.FromRemoved(&cStack, false));
		AssertFalse(sFreedNotifierStack.bFreed);
		cStack.Kill();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object with a Ptr field its Class() leaves out.
//
//////////////////////////////////////////////////////////////////////////
class CTestPtrNotInClass : public CObject
{
CONSTRUCTABLE(CTestPtrNotInClass)
DESTRUCTABLE(CTestPtrNotInClass)
public:
	Ptr<CTestObject>	mpMissing;

	void Init(void)
	{
		PreInit();
		PostInit();
	}

	void Class(void) override
	{
	}

	void Free(void) override
	{
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A map with a Ptr field its Class() leaves out.
//
//////////////////////////////////////////////////////////////////////////
class CTestMapPtrNotInClass : public CMapObject
{
CONSTRUCTABLE(CTestMapPtrNotInClass)
DESTRUCTABLE(CTestMapPtrNotInClass)
public:
	Ptr<CTestObject>	mpMissing;

	void Class(void) override
	{
		CMapObject::Class();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A Ptr field missing from an object's Class() is a stack pointer inside
// the object, and validating finds it, among many stack pointers (and in
// a map, which checks its own state too).  Stack pointers that aren't
// inside an object aren't reported.
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersInsideObject(void)
{
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLogged[2048];

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject;
		Ptr<CTestObject>			apHolders[50];
		Ptr<CTestPtrNotInClass>		pHolder;
		Ptr<CTestMapPtrNotInClass>	pMap;
		size						i;

		pObject = OMalloc<CTestObject>();
		for (i = 0; i < 50; i++)
		{
			apHolders[i] = pObject;
		}

		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateObjectsConsistency();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertString("", szLogged);

		pHolder = OMalloc<CTestPtrNotInClass>();
		pMap = OMalloc<CTestMapPtrNotInClass>();
		pHolder->mpMissing = pObject;
		pMap->mpMissing = pObject;

		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateObjectsConsistency();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertTrue(strstr(szLogged, "appears to belong to Object {") != NULL);
		AssertTrue(strstr(szLogged, "CTestPtrNotInClass") != NULL);
		AssertTrue(strstr(szLogged, "CTestMapPtrNotInClass") != NULL);

		pMap->mpMissing = NULL;
		pHolder->mpMissing = NULL;
		StartCapturingLog(&cLog, &sLogConfig);
		gcObjects.ValidateObjectsConsistency();
		StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
		AssertTrue(strstr(szLogged, "appears to belong to Object {") == NULL);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointers(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestStackPointersAdd();
	TestStackPointersNewestFirst();
	TestStackPointersKillSingle();
	TestStackPointersHeapKillSingle();
	TestStackPointersHeapKill();
	TestStackPointersSorting1();
	TestStackPointersRemoveMissing();
	TestStackPointersFromRemoved();
	TestStackPointersInsideObject();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

