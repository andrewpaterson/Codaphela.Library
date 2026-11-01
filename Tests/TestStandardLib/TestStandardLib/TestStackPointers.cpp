#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/StackPointers.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointersAdd(void)
{
	CStackPointers	cStackPointes;
	CPointer		cPointer1;
	CPointer		cPointer2;
	CStackPointer*	pcStackPointer11;
	CStackPointer*	pcStackPointer12;
	CStackPointer*	pcStackPointer13;
	CStackPointer*	pcStackPointer21;
	CStackPointer*	pcStackPointer22;
	CStackPointer*	pcStackPointer23;

	cStackPointes.Init();

	pcStackPointer11 = cStackPointes.Add(cPointer1.This());
	AssertNotNull(pcStackPointer11);

	pcStackPointer12 = cStackPointes.Add(cPointer1.This(), pcStackPointer11);
	AssertNotNull(pcStackPointer12);
	AssertPointer(pcStackPointer11->GetNext(), pcStackPointer12);

	AssertInt(2, pcStackPointer11->NumPointers());

	pcStackPointer21 = cStackPointes.Add(cPointer2.This());
	AssertNotNull(pcStackPointer21);

	pcStackPointer22 = cStackPointes.Add(cPointer2.This(), pcStackPointer21);
	AssertNotNull(pcStackPointer22);
	AssertPointer(pcStackPointer21->GetNext(), pcStackPointer22);

	AssertInt(2, pcStackPointer21->NumPointers());

	AssertSize(4, cStackPointes.NumElements());

	pcStackPointer13 = cStackPointes.Add(cPointer1.This(), pcStackPointer11);
	AssertNotNull(pcStackPointer13);
	AssertPointer(pcStackPointer11->GetNext(), pcStackPointer12);
	AssertPointer(pcStackPointer12->GetNext(), pcStackPointer13);
	AssertNull(pcStackPointer13->GetNext());

	AssertInt(3, pcStackPointer11->NumPointers());

	pcStackPointer23 = cStackPointes.Add(cPointer1.This(), pcStackPointer21);
	AssertNotNull(pcStackPointer23);
	AssertPointer(pcStackPointer21->GetNext(), pcStackPointer22);
	AssertPointer(pcStackPointer22->GetNext(), pcStackPointer23);
	AssertNull(pcStackPointer23->GetNext());

	AssertInt(3, pcStackPointer21->NumPointers());

	AssertSize(6, cStackPointes.NumElements());
	
	cStackPointes.RemoveAll(pcStackPointer21);

	AssertSize(3, cStackPointes.NumElements());

	pcStackPointer21 = cStackPointes.Add(cPointer1.This());
	AssertNotNull(pcStackPointer21);

	pcStackPointer22 = cStackPointes.Add(cPointer2.This(), pcStackPointer21);
	AssertNotNull(pcStackPointer22);
	AssertPointer(pcStackPointer21->GetNext(), pcStackPointer22);

	AssertInt(2, pcStackPointer21->NumPointers());

	pcStackPointer23 = cStackPointes.Add(cPointer1.This(), pcStackPointer21);
	AssertNotNull(pcStackPointer23);
	AssertPointer(pcStackPointer21->GetNext(), pcStackPointer22);
	AssertPointer(pcStackPointer22->GetNext(), pcStackPointer23);
	AssertNull(pcStackPointer23->GetNext());

	AssertInt(3, pcStackPointer21->NumPointers());

	AssertSize(6, cStackPointes.NumElements());

	cStackPointes.Kill();
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

	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest2->GetDistToStack());
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
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest2->GetDistToStack());
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
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest2->GetDistToStack());
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
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest2->GetDistToStack());
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
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->mpTest->GetDistToStack());
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

	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
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

	AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
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
		CTestEmbeddedObjectWithFields*		pcTest1;
		CTestEmbeddedObjectWithFields*		pcTest2;
		CTestEmbeddedObjectWithFields*		pcTest3;
		CTestEmbeddedObjectWithFields*		pcTest4;
		CTestEmbeddedObjectWithFields*		pcTest5;
		CTestEmbeddedObjectWithFields*		pcTest6;

		pTest1 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest2 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest3 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest4 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest5 = OMalloc<CTestEmbeddedObjectWithFields>();
		pTest6 = OMalloc<CTestEmbeddedObjectWithFields>();
		pcTest1 = &pTest1;
		pcTest2 = &pTest2;
		pcTest3 = &pTest3;
		pcTest4 = &pTest4;
		pcTest5 = &pTest5;
		pcTest6 = &pTest6;

		AssertInt(UNKNOWN_DIST_TO_STACK, pTest1->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pTest2->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pTest3->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pTest4->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pTest5->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pTest6->GetDistToStack());

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
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest6->GetDistToStack());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		pcTest5->mpObjectA = NULL;
		AssertSize(5, gcObjects.NumMemoryIndexes());

		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());
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
//
//////////////////////////////////////////////////////////////////////////
void TestStackPointers(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestStackPointersAdd();
	TestStackPointersKillSingle();
	TestStackPointersHeapKillSingle();
	TestStackPointersHeapKill();
	TestStackPointersSorting1();
	TestStackPointersRemoveMissing();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

