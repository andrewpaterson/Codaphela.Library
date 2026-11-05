#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Root.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/Set.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSimpleOneStep(void)
{
	ObjectsInit();

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");

	AssertSize(0, p2->NumHeapFroms());
	AssertSize(0, p1->NumHeapFroms());
	AssertSize(0, p3->NumHeapFroms());

	//  
	//  x   x   x
	//  |   |   |
	//  |   |   |
	//  p1  p2  p3
	//  .   .   .
	//  .   .   .
	//

	p3->mp = p2;
	AssertSize(1, p2->NumHeapFroms());

	//  
	//      x
	//      |
	//      |
	//  x   p2
	//  |   . \
	//  |   .  \
	//  p1  .   p3
	//  .   .   .
	//  .   .   .
	//

	p3 = NULL;
	AssertSize(0, p2->NumHeapFroms());

	//  
	//  x   x 
	//  |   | 
	//  |   | 
	//  p1  p2
	//  .   . 
	//  .   . 
	//

	gcObjects.ValidateObjectsConsistency();

	p1 = NULL;
	p2 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSimpleTwoStep(void)
{
	ObjectsInit();

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");
	AssertInt(3, (int)gcObjects.NumMemoryIndexes());
	AssertSize(0, p2->NumHeapFroms());

	//
	//     x
	//     |
	//     |
	//     p2      x 
	//    /  .     | 
	//   /    .    | 
	//  p3	  .    p1
	//  . 	  .    .  
	//  . 	  .    . 
	//

	p3->mp = p2;
	AssertSize(1, p2->NumHeapFroms());

	p3->mp = NULL;

	//  
	//  x   x   x
	//  |   |   |
	//  |   |   |
	//  p1  p2  p3
	//  .   .   .
	//  .   .   .
	//

	AssertSize(0, p2->NumHeapFroms());
	AssertInt(3, (int)gcObjects.NumMemoryIndexes());
	p3 = NULL;

	//  
	//  x   x 
	//  |   | 
	//  |   | 
	//  p1  p2
	//  .   . 
	//  .   . 
	//

	AssertInt(2, (int)gcObjects.NumMemoryIndexes());
	AssertTrue(gcObjects.Contains("Pointer A"));
	AssertTrue(gcObjects.Contains("Pointer B"));

	gcObjects.ValidateObjectsConsistency();

	p1 = NULL;
	p2 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSimpleHeap(void)
{
	ObjectsInit();

	CPointerContainer*			pc2;
	CPointerContainer*			pc3;

	CDistCalculator				cDistCalculator;

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");

	AssertInt(3, (int)gcObjects.NumMemoryIndexes());
	AssertSize(0, p2->NumHeapFroms());

	AssertSize(1, p2->NumStackFroms());
	AssertSize(1, p1->NumStackFroms());
	AssertSize(1, p3->NumStackFroms());

	//
	//     x
	//     |
	//     |
	//     p2      x 
	//    /  .     | 
	//   /    .    | 
	//  p3	  .    p1
	//  . 	  .    .  
	//  . 	  .    . 
	//

	p3->mp = p2;
	AssertSize(1, p2->NumHeapFroms());
	AssertSize(1, p2->NumStackFroms());

	pc3 = &p3;
	pc2 = &p2;


	p2.BaseObject()->TestRemoveStackFrom(p2.This());
	p2.UnsafeClearObject();

	AssertSize(1, pc2->NumHeapFroms());
	AssertSize(0, pc2->NumStackFroms());
	AssertSize(1, p1->NumStackFroms());
	AssertSize(1, p3->NumStackFroms());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pc2);

	//
	//  x
	//  |
	//  |
	//  p2     x 
	//  |      | 
	//  |      | 
	//  p3	   p1
	//  . 	   . 
	//  . 	   . 
	//

	AssertTrue(cDistCalculator.IsReachedFromStack(pc2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pc3));

	cDistCalculator.Kill();

	p1 = NULL;
	p2 = NULL;
	p3 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackRootCyclicWithStackPointerA(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	CPointerContainer*			pc1;
	CPointerContainer*			pc2;
	CPointerContainer*			pc3;

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");
	p1->mp = p2;
	p2->mp = p3;
	p3->mp = p1;

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CPointerContainer>	p0 = ONMalloc<CPointerContainer>("Test Helper", p1);
	pRoot->Add(p0);

	Ptr<CPointerContainer> pStack = OMalloc<CPointerContainer>(p2);

	AssertInt(2, p0->GetDistToRoot());
	AssertInt(3, p1->GetDistToRoot());
	AssertInt(4, p2->GetDistToRoot());
	AssertInt(5, p3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	pStack = p1;
	pc1 = &p1;
	pc2 = &p2;
	pc3 = &p3;
	p1 = NULL;
	p2 = NULL;
	p3 = NULL;

	gcObjects.ValidateObjectsConsistency();

	p0->mp.UnsafeClearObject();
	pc1->TestRemoveHeapFrom(p0.BaseObject());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pc1);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	cDistCalculator.CalculateFromTouched();

	AssertSize(3, cDistCalculator.NumDetachedFromRoot());
	AssertSize(0, cDistCalculator.NumCompletelyDetached());

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	p0 = NULL;
	pRoot = NULL;
	pStack = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackRootCyclicWithStackPointerB(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	CPointerContainer*			pc1;
	CPointerContainer*			pc2;
	CPointerContainer*			pc3;

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");
	p1->mp = p2;
	p2->mp = p3;
	p3->mp = p1;

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CPointerContainer>	p0 = ONMalloc<CPointerContainer>("Test Helper", p1);
	pRoot->Add(p0);

	Ptr<CPointerContainer> pStack = OMalloc<CPointerContainer>(p1);

	AssertInt(2, p0->GetDistToRoot());
	AssertInt(3, p1->GetDistToRoot());
	AssertInt(4, p2->GetDistToRoot());
	AssertInt(5, p3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	pStack = p1;
	pc1 = &p1;
	pc2 = &p2;
	pc3 = &p3;
	p1 = NULL;
	p2 = NULL;
	p3 = NULL;

	p0->mp.UnsafeClearObject();
	pc1->TestRemoveHeapFrom(p0.BaseObject());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pc1);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	cDistCalculator.CalculateFromTouched();

	AssertSize(3, cDistCalculator.NumDetachedFromRoot());
	AssertSize(0, cDistCalculator.NumCompletelyDetached());

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	p0 = NULL;
	pRoot = NULL;
	pStack = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackRootCyclicWithStackPointerC(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	CPointerContainer*			pc1;
	CPointerContainer*			pc2;
	CPointerContainer*			pc3;

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");
	p1->mp = p2;
	p2->mp = p3;
	p3->mp = p1;

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CPointerContainer>	p0 = ONMalloc<CPointerContainer>("Test Helper", p1);
	pRoot->Add(p0);

	Ptr<CPointerContainer> pStack = OMalloc<CPointerContainer>(p3);

	AssertInt(2, p0->GetDistToRoot());
	AssertInt(3, p1->GetDistToRoot());
	AssertInt(4, p2->GetDistToRoot());
	AssertInt(5, p3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	pStack = p1;
	pc1 = &p1;
	pc2 = &p2;
	pc3 = &p3;
	p1 = NULL;
	p2 = NULL;
	p3 = NULL;

	p0->mp.UnsafeClearObject();
	pc1->TestRemoveHeapFrom(p0.BaseObject());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pc1);
	
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());
	
	cDistCalculator.CalculateFromTouched();

	AssertSize(3, cDistCalculator.NumDetachedFromRoot());
	AssertSize(0, cDistCalculator.NumCompletelyDetached());

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	p0 = NULL;
	pRoot = NULL;
	pStack = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackRootCyclicWithoutStackPointer(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	CPointerContainer*			pc1;
	CPointerContainer*			pc2;
	CPointerContainer*			pc3;

	Ptr<CPointerContainer>	p1 = ONMalloc<CPointerContainer>("Pointer A");
	Ptr<CPointerContainer>	p2 = ONMalloc<CPointerContainer>("Pointer B");
	Ptr<CPointerContainer>	p3 = ONMalloc<CPointerContainer>("Pointer C");
	p1->mp = p2;
	p2->mp = p3;
	p3->mp = p1;

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CPointerContainer>	p0 = ONMalloc<CPointerContainer>("Test Helper", p1);
	pRoot->Add(p0);

	//
	// p2 --> p3
	//  ^     /
	//   \   /
	//    \ v
	//    p1
	//     ^
	//     |
	//     |
	//    p0
	//     ^
	//     |
	//     |
	//    ...
	//   pRoot
	//

	AssertInt(2, p0->GetDistToRoot());
	AssertInt(3, p1->GetDistToRoot());
	AssertInt(4, p2->GetDistToRoot());
	AssertInt(5, p3->GetDistToRoot());

	pc1 = &p1;
	pc2 = &p2;
	pc3 = &p3;
	p1 = NULL;
	p2 = NULL;
	p3 = NULL;

	AssertSize(2, pc1->NumHeapFroms());
	AssertSize(0, pc1->NumStackFroms());

	p0->mp.UnsafeClearObject();
	pc1->TestRemoveHeapFrom(p0.BaseObject());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pc1);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	
	cDistCalculator.CalculateFromTouched();

	AssertSize(3, cDistCalculator.NumDetachedFromRoot());
	AssertSize(3, cDistCalculator.NumCompletelyDetached());
	AssertPointer(pc1, cDistCalculator.GetCompletelyDetached(0));
	AssertPointer(pc3, cDistCalculator.GetCompletelyDetached(1));
	AssertPointer(pc2, cDistCalculator.GetCompletelyDetached(2));

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	p0 = NULL;
	pRoot = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSplitRootAndStack(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	CTestObject*				pcTestC;
	CTestObject*				pcTestL1;
	CTestObject*				pcTestL2;
	CTestObject*				pcTestR1;
	CTestObject*				pcTestR2;
	CPointerContainer*			pcC;

	Ptr<CTestObject>	pTestC = OMalloc<CTestObject>();
	Ptr<CTestObject>	pTestL1 = OMalloc<CTestObject>();
	Ptr<CTestObject>	pTestL2 = OMalloc<CTestObject>();
	Ptr<CTestObject>	pTestR1 = OMalloc<CTestObject>();
	Ptr<CTestObject>	pTestR2 = OMalloc<CTestObject>();

	pTestL1->mpTest = pTestL2;
	pTestR1->mpTest = pTestR2;
	pTestC->mpTest = pTestL1;
	pTestC->mpObject = pTestR1;

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CPointerContainer>	pC = ONMalloc<CPointerContainer>("Test Helper Center", pTestC);
	Ptr<CPointerContainer>	pL1 = ONMalloc<CPointerContainer>("Test Helper Left", pTestL1);
	pRoot->Add(pC);
	pRoot->Add(pL1);

	Ptr<CPointerContainer> pStack = OMalloc<CPointerContainer>();
	pStack = pTestR1;

	AssertInt(3, pTestC->GetDistToRoot());
	AssertInt(3, pTestL1->GetDistToRoot());
	AssertInt(4, pTestR1->GetDistToRoot());
	AssertInt(4, pTestL2->GetDistToRoot());
	AssertInt(5, pTestR2->GetDistToRoot());

	pcTestC = &pTestC;
	pcTestL1 = &pTestL1;
	pcTestL2 = &pTestL2;
	pcTestR1 = &pTestR1;
	pcTestR2 = &pTestR2;
	pcC = &pC;

	pTestC = NULL;
	pTestL1 = NULL;
	pTestL2 = NULL;
	pTestR1 = NULL;
	pTestR2 = NULL;
	pC = NULL;
	pL1 = NULL;

	pcC->mp.UnsafeClearObject();
	pcTestC->TestRemoveHeapFrom(pcC);

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pcTestC);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestC->GetDistToRoot());
	AssertInt(3, pcTestL1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestR1->GetDistToRoot());
	AssertInt(4, pcTestL2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestR2->GetDistToRoot());

	AssertSize(3, cDistCalculator.NumTouched());

	cDistCalculator.CalculateFromTouched();

	AssertSize(3, cDistCalculator.NumDetachedFromRoot());
	AssertSize(1, cDistCalculator.NumCompletelyDetached());
	AssertPointer(pcTestC, cDistCalculator.GetCompletelyDetached(0));


	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	pRoot = NULL;
	pStack = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSetWithoutStackPointers(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	STestObjectFreedNotifier	sFreedNotifier;
	Ptr<CSet<CTestObject>>		pSet;
	Ptr<CRoot>					pRoot;
	Ptr<CPointerContainer>		pContainer;
	Ptr<CTestObject>			pTest;
	CSet<CTestObject>*			pcSet;
	CPointerContainer*			pcContainer;
	CTestObject*				pcTest;

	pTest = OMalloc<CTestObject>(&sFreedNotifier);
	pSet = OMalloc<CSet<CTestObject>>(false);
	pSet->Add(pTest);

	pContainer = OMalloc<CPointerContainer>();
	pContainer->mp = pSet;
	pRoot = ORoot();
	pRoot->Add(pContainer);

	pcSet = &pSet;
	pcContainer = &pContainer;
	pcTest = &pTest;
	pSet = NULL;
	pContainer = NULL;
	pTest = NULL;

	pcContainer->mp.UnsafeClearObject();
	pcSet->TestRemoveHeapFrom(pcContainer);

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pcSet);
	cDistCalculator.CalculateFromTouched();

	AssertSize(2, cDistCalculator.NumDetachedFromRoot());
	AssertSize(2, cDistCalculator.NumCompletelyDetached());
	AssertPointer(pcSet, cDistCalculator.GetCompletelyDetached(0));
	AssertPointer(pcTest, cDistCalculator.GetCompletelyDetached(1));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistCalculator.GetCompletelyDetachedArray());
	AssertLong(3, gcObjects.NumMemoryIndexes());

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	pRoot = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSetWithStackPointers(void)
{
	ObjectsInit();

	CDistCalculator	cDistCalculator;
	STestObjectFreedNotifier	sFreedNotifier;
	Ptr<CSet<CTestObject>>		pSet;
	Ptr<CRoot>					pRoot;
	Ptr<CPointerContainer>		pContainer;
	Ptr<CTestObject>			pTest;
	CSet<CTestObject>*			pcSet;
	CPointerContainer*			pcContainer;

	pTest = OMalloc<CTestObject>(&sFreedNotifier);
	pSet = OMalloc<CSet<CTestObject>>(false);
	pSet->Add(pTest);

	pContainer = OMalloc<CPointerContainer>();
	pContainer->mp = pSet;
	pRoot = ORoot();
	pRoot->Add(pContainer);

	pcSet = &pSet;
	pcContainer = &pContainer;
	pSet = NULL;
	pContainer = NULL;

	pcContainer->mp.UnsafeClearObject();
	pcSet->TestRemoveHeapFrom(pcContainer);

	cDistCalculator.Init();
	
	cDistCalculator.CalculateDistToRoot(pcSet);
	cDistCalculator.CalculateFromTouched();

	AssertSize(2, cDistCalculator.NumDetachedFromRoot());
	AssertSize(1, cDistCalculator.NumCompletelyDetached());
	AssertPointer(pcSet, cDistCalculator.GetCompletelyDetached(0));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistCalculator.GetCompletelyDetachedArray());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	cDistCalculator.Kill();

	gcObjects.ValidateObjectsConsistency();

	pTest = NULL;
	pRoot = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackSetBroken(void)
{
	CDistCalculator	cDistCalculator;

	STestObjectFreedNotifier	sKilled1;
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;

	Ptr<CRoot>					pRoot;
	Ptr<CTestObject>			pTest1;
	Ptr<CTestObject>			pTest2;
	Ptr<CTestObject>			pTest3;

	CObject*					pcTest2;
	CObject*					pcTest3;
	CSetObject*					pcSet;

	ObjectsInit();

	pTest1 = OMalloc<CTestObject>(&sKilled1);
	pTest3 = OMalloc<CTestObject>(&sKilled3);

	pRoot = ORoot();

	pRoot->Add(pTest1);
	pRoot->Add(pTest3);

	pTest2 = OMalloc<CTestObject>(&sKilled2);

	pTest1->mpTest = pTest2;
	AssertInt(3, pTest2->GetDistToRoot());

	pTest3->mpTest = pTest2;
	AssertInt(3, pTest2->GetDistToRoot());

	//           
	//           pTest2[5](4)
	//            ^ ^
	//           /   \
	//          /     \
	//         /       \
	//   pTest1[1](3)  pTest3[2](3)
	//         ^       ^
	//          \     /
	//           \   /
	//            \ /
	//            ...
	//           pRoot[3](0) 
	//    

	pcTest3 = &pTest3;
	pcTest2 = &pTest2;
	pTest1 = NULL;
	pTest2 = NULL;
	pTest3 = NULL;

	AssertLong(5, gcObjects.NumMemoryIndexes());
	AssertInt(3, pcTest2->GetDistToRoot());
	AssertSize(2, pcTest2->NumHeapFroms());
	
	pcSet = pRoot->TestGetSet();
	pcSet->UnsafeRemove(pcTest3);
	pcTest3->TestRemoveHeapFrom(pcSet);

	AssertSize(2, pcTest2->NumHeapFroms());

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToRoot(pcTest3);
	AssertInt(3, pcTest2->GetDistToRoot());

	cDistCalculator.CalculateFromTouched();

	AssertSize(1, cDistCalculator.NumDetachedFromRoot());
	AssertSize(1, cDistCalculator.NumCompletelyDetached());
	AssertPointer(pcTest3, cDistCalculator.GetCompletelyDetached(0));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistCalculator.GetCompletelyDetachedArray());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	cDistCalculator.Kill();

	AssertFalse(sKilled1.bFreed);
	AssertTrue( sKilled3.bFreed);
	AssertFalse(sKilled2.bFreed);

	gcObjects.ValidateObjectsConsistency();

	pRoot = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackNoStackHeapFromBroken(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	CTestObject*				pcTest1;
	CTestObject*				pcTest2;

	Ptr<CTestObject> pTest1 = OMalloc<CTestObject>(&sNotifier1);
	Ptr<CTestObject> pTest2 = OMalloc<CTestObject>(&sNotifier2);

	pTest2->mpTest = pTest1;

	//
	//    pTest1
	//      ^ ^
	//      |  .
	//      |   .
	//      |
	//    pTest2
	//        ^
	//         .
	//          .
	//

	pcTest2 = (CTestObject*)pTest2.Object();
	pcTest1 = (CTestObject*)pTest1.Object();

	pTest1 = NULL;  //pTest1 should not be killed here, even though it has no stack froms.  
					//It still has a heap from that has a stack from.

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
void TestDistToStackNoHeapA(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	CTestObject*				pcTest1;
	CTestObject*				pcTest2;
	CTestObject					cTest1;

	Ptr<CTestObject> pTest2 = OMalloc<CTestObject>(&sNotifier2);
	
	cTest1.Init(&sNotifier1);
	pTest2->mpTest = &cTest1;

	//
	//    cTest1
	//      ^ 
	//      |  
	//      |   
	//      |
	//    pTest2
	//        ^
	//         .
	//          .
	//

	AssertFalse(pTest2->mpTest.IsNull());
	AssertTrue(cTest1.mpTest.IsNull());

	pcTest2 = (CTestObject*)pTest2.Object();
	pcTest1 = &cTest1;

	AssertLong(INVALID_O_INDEX, pcTest1->GetIndex());
	AssertLong(1, pcTest2->GetIndex());

	cTest1.Kill();

	AssertTrue(sNotifier1.bFreed);
	AssertFalse(sNotifier2.bFreed);
	Pass();

	AssertTrue(pTest2->mpTest.IsNull());
	Pass();

	AssertSize(0, pcTest1->NumHeapFroms());
	AssertSize(0, pcTest1->NumStackFroms());
	AssertSize(0, pcTest1->NumPointerTos());
	AssertSize(0, pcTest2->NumHeapFroms());
	AssertSize(1, pcTest2->NumStackFroms());
	AssertSize(0, pcTest2->NumPointerTos());

	pTest2 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackNoHeapB(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	CTestObject*				pcTest1;
	CTestObject*				pcTest2;
	CTestObject					cTest1;

	Ptr<CTestObject> pTest2 = OMalloc<CTestObject>(&sNotifier2);

	cTest1.Init(&sNotifier1);
	cTest1.mpTest = pTest2;

	//
	//    pTest2
	//      ^ ^ 
	//      |  .
	//      |   .
	//      |    .
	//    cTest1
	//

	AssertTrue(pTest2->mpTest.IsNull());
	AssertFalse(cTest1.mpTest.IsNull());
	Pass();

	pcTest2 = (CTestObject*)pTest2.Object();
	pcTest1 = &cTest1;

	AssertLong(INVALID_O_INDEX, pcTest1->GetIndex());
	AssertLong(1, pcTest2->GetIndex());

	cTest1.Kill();

	AssertTrue(sNotifier1.bFreed);
	AssertFalse(sNotifier2.bFreed);
	Pass();

	AssertSize(0, pcTest1->NumHeapFroms());
	AssertSize(0, pcTest1->NumStackFroms());
	AssertSize(0, pcTest1->NumPointerTos());
	AssertSize(0, pcTest2->NumHeapFroms());
	AssertSize(1, pcTest2->NumStackFroms());
	AssertSize(0, pcTest2->NumPointerTos());

	pTest2 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackNoHeapC(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifier1;
	STestObjectFreedNotifier	sNotifier2;
	CTestObject*				pcTest1;
	CTestObject*				pcTest2;
	CTestObject					cTest1;

	Ptr<CTestObject> pTest2 = OMalloc<CTestObject>(&sNotifier2);

	cTest1.Init(&sNotifier1);
	cTest1.mpTest = pTest2;

	//
	//    pTest2
	//      ^ ^ 
	//      |  .
	//      |   .
	//      |    .
	//    cTest1
	//

	AssertTrue(pTest2->mpTest.IsNull());
	AssertFalse(cTest1.mpTest.IsNull());
	Pass();

	pcTest2 = (CTestObject*)pTest2.Object();
	pcTest1 = &cTest1;

	AssertLong(INVALID_O_INDEX, pcTest1->GetIndex());
	AssertLong(1, pcTest2->GetIndex());

	pTest2->Kill();

	AssertFalse(sNotifier1.bFreed);
	AssertTrue(sNotifier2.bFreed);
	Pass();

	AssertTrue(cTest1.mpTest.IsNull());
	Pass();

	AssertSize(0, pcTest1->NumHeapFroms());
	AssertSize(0, pcTest1->NumStackFroms());
	AssertSize(0, pcTest1->NumPointerTos());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackCyclicWithScenarioA(void)
{
	ObjectsInit();

	CTestObject*				pcTest1;
	CTestTriPointerObject*		pcTest2;
	CTestTriPointerObject*		pcTest3;
	CTestObject*				pcTest4;
	CTestObject*				pcTest5;

	CDistCalculator				cDistCalculator;

	Ptr<CTestObject>			pTest1 = OMalloc<CTestObject>();
	Ptr<CTestTriPointerObject>	pTest2 = OMalloc<CTestTriPointerObject>();
	Ptr<CTestTriPointerObject>	pTest3 = OMalloc<CTestTriPointerObject>();
	Ptr<CTestObject>			pTest4 = OMalloc<CTestObject>();
	Ptr<CTestObject>			pTest5 = OMalloc<CTestObject>();

	pcTest1 = &pTest1;
	pcTest2 = &pTest2;
	pcTest3 = &pTest3;
	pcTest4 = &pTest4;
	pcTest5 = &pTest5;

	pTest1->mpTest = pTest2;
	pTest1->mpObject = pTest3;
	pTest2->mpObject1 = pTest1;
	pTest2->mpObject2 = pTest3;
	pTest2->mpObject3 = pTest4;
	pTest3->mpObject1 = pTest1;
	pTest3->mpObject2 = pTest2;
	pTest3->mpObject3 = pTest5;

	pcTest2->TestRemoveStackFrom(pTest2.This());
	pcTest3->TestRemoveStackFrom(pTest3.This());
	pcTest4->TestRemoveStackFrom(pTest4.This());
	pcTest5->TestRemoveStackFrom(pTest5.This());
	pTest2.UnsafeClearObject();
	pTest3.UnsafeClearObject();
	pTest4.UnsafeClearObject();
	pTest5.UnsafeClearObject();

	//
	//  pTest4   pTest5
	//    |         |
	//    |         |
	//  pTest2===pTest3
	//     \\     //
	//      \\   //
	//       \\ //
	//       pTest1
	//         . 
	//         . 
	//

	// Test from pTest1

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pcTest1);
	
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest1));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest3));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest4));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest5));

	cDistCalculator.Kill();

	// Test from pTest2

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pcTest2);

	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest1));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest3));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest4));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest5));

	cDistCalculator.Kill();

	// Test from pTest3

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pcTest3);

	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest1));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest3));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest4));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest5));

	cDistCalculator.Kill();

	// Test from pTest4

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pcTest4);

	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest1));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest3));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest4));
	AssertFalse(cDistCalculator.IsReachedFromStack(pcTest5));  //Removing a stack pointer to 4 should be unable to effect 5 in any way.

	cDistCalculator.Kill();

	// Test from pTest5

	cDistCalculator.Init();
	cDistCalculator.CalculateDistToStack(pcTest5);

	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest1));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest2));
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest3));
	AssertFalse(cDistCalculator.IsReachedFromStack(pcTest4));  //Removing a stack pointer to 5 should be unable to effect 4 in any way.
	AssertTrue(cDistCalculator.IsReachedFromStack(pcTest5));

	cDistCalculator.Kill();

	pTest1 = NULL;
	pTest2 = NULL;
	pTest3 = NULL;
	pTest4 = NULL;
	pTest5 = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackRemoveUnbalancedLargeBroken(void)
{
	STestObjectFreedNotifier	sKilled1;
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;
	STestObjectFreedNotifier	sKilled4;
	STestObjectFreedNotifier	sKilled5;
	STestObjectFreedNotifier	sKilledTop1;
	STestObjectFreedNotifier	sKilledTop2;

	ObjectsInit();
	{
		CDistCalculator	cDistCalculator;

		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CSetObject>				pSet;
		Ptr<CTestObject>			pTop1;
		Ptr<CTestObject>			pTop2;
		CTestObject* pcTest2;
		CTestObject* pcTest3;
		CTestObject* pcTest4;
		CTestObject* pcTest5;
		CArrayBaseObjectPtr* papcKilled;

		pRoot = ORoot();

		pTest1 = OMalloc<CTestObject>(&sKilled1);
		pTest2 = OMalloc<CTestObject>(&sKilled2);
		pTest3 = OMalloc<CTestObject>(&sKilled3);
		pTest4 = OMalloc<CTestObject>(&sKilled4);
		pTest5 = OMalloc<CTestObject>(&sKilled5);
		pSet = OMalloc<CSetObject>(false);
		pTop1 = OMalloc<CTestObject>(&sKilledTop1);
		pTop2 = OMalloc<CTestObject>(&sKilledTop2);

		pRoot->Add(pSet);
		pSet->Add(pTest2);
		pSet->Add(pTop2);

		pTest2->mpTest = pTest3;
		pTest5->mpTest = pTop1;
		pTest4->mpTest = pTest5;
		pTest3->mpTest = pTest4;
		pTop1->mpTest = pTest1;
		pTop2->mpTest = pTest1;
		pTest1->mpTest = pTop1;
		pTest1->mpObject = pTop2;

		AssertInt(2, pSet->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(4, pTest3->GetDistToRoot());
		AssertInt(5, pTest4->GetDistToRoot());
		AssertInt(6, pTest5->GetDistToRoot());
		AssertInt(4, pTest1->GetDistToRoot());
		AssertInt(5, pTop1->GetDistToRoot());
		AssertInt(3, pTop2->GetDistToRoot());
		AssertSize(2, pSet->NumElements());
		AssertSize(2, pSet->NumPointerTos());

		pcTest2 = &pTest2;
		pcTest3 = &pTest3;
		pcTest4 = &pTest4;
		pcTest5 = &pTest5;

		pTest3 = NULL;
		pTest4 = NULL;
		pTest5 = NULL;

		//
		//     Test1[3](4)  
		//       // \\      
		//      //   \\       
		//   Top1(5) Top2(3)
		//     |       |
		//   Test5(6)  |
		//     |       |
		//   Test4(5)  |
		//     |       |
		//   Test3(4)  |
		//     |	   |
		// Test2[4](3) |
		//      \     /
		//       \   /
		//       Set(2)
		//        |
		//       ...
		//      Root(0)
		//


		pSet->Remove(pcTest2);


		gcObjects.ValidateObjectsConsistency();

		pcTest2->TestRemoveStackFrom(pTest2.This());
		pTest2.UnsafeClearObject();

		cDistCalculator.Init();
		cDistCalculator.CalculateDistToStack(pcTest2);

		papcKilled = cDistCalculator.GetCompletelyDetachedArray();
		AssertSize(4, papcKilled->NumElements());

		cDistCalculator.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object pointed to only into one of its embedded objects, from an
// object reached from the stack, is reached too.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackReachedThroughEmbedded(void)
{
	CDistCalculator	cDistCalculator;

	ObjectsInit();
	{
		Ptr<CEmbeddedContainer>	pContainer;
		Ptr<CPointerContainer>	pNear;
		CBaseObject*			pcContainer;

		pContainer = OMalloc<CEmbeddedContainer>();
		pNear = OMalloc<CPointerContainer>();

		//
		//  Near
		//    |
		//  mcOne
		//    |
		//  Container
		//

		pNear->mp = pContainer->GetOne();
		AssertSize(1, pContainer->NumHeapFroms());  //Its embedded objects' froms too.
		AssertSize(1, pContainer->mcOne.NumHeapFroms());

		//Held by Near, so letting go of it frees nothing.
		pcContainer = &pContainer;
		pContainer = NULL;
		AssertSize(0, pcContainer->NumStackFroms());

		cDistCalculator.Init();
		cDistCalculator.CalculateDistToStack(pcContainer);
		AssertTrue(cDistCalculator.IsReachedFromStack(pcContainer));
		AssertTrue(cDistCalculator.IsReachedFromStack(&pNear));
		AssertSize(0, cDistCalculator.NumCompletelyDetached());
		cDistCalculator.Kill();

		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object is held when something pointing to it (or to what points to
// it...) is pointed to from the stack or attached to the root; never by
// itself.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackIsHeldFromStackOrRoot(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CPointerContainer>		pA;
		Ptr<CPointerContainer>		pB;
		Ptr<CPointerContainer>		pC;
		CBaseObject*				pcA;
		CBaseObject*				pcB;

		pRoot = ORoot();
		pA = OMalloc<CPointerContainer>();
		pB = OMalloc<CPointerContainer>();
		pcA = &pA;
		pcB = &pB;

		//  B -> A, B on the stack.
		pB->mp = pA;
		AssertTrue(CDistCalculator::IsHeldFromStackOrRoot(pcA));

		//  A <-> B, only A on the stack.  B is held through A; A only by itself.
		pA->mp = pB;
		pB = NULL;
		AssertTrue(CDistCalculator::IsHeldFromStackOrRoot(pcB));
		AssertFalse(CDistCalculator::IsHeldFromStackOrRoot(pcA));

		//  Root -> C -> A.  A is held through the root.
		pC = OMalloc<CPointerContainer>();
		pRoot->Add(pC);
		pC->mp = pA;
		pC = NULL;
		AssertTrue(CDistCalculator::IsHeldFromStackOrRoot(pcA));

		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Distance calculations either side of the walk count wrapping free what
// they should: the stamps from before the wrap are cleared, not mistaken
// for a new walk's.
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackAcrossWalkStampWrap(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CPointerContainer>		pA;
		Ptr<CPointerContainer>		pB;
		size						uiRootOnly;

		pRoot = ORoot();
		uiRootOnly = gcObjects.NumMemoryIndexes();
		pA = OMalloc<CPointerContainer>();
		pB = OMalloc<CPointerContainer>();
		pA->mp = pB;
		pB->mp = pA;
		pRoot->Add(pA);

		gcObjects.TestSetWalkStamp(0xFFFFFFF0);

		//Detached from the root, held from the stack (the last walks before the wrap).
		pRoot->Remove(pA);
		AssertSize(uiRootOnly + 2, gcObjects.NumMemoryIndexes());
		pA = NULL;
		AssertSize(uiRootOnly + 2, gcObjects.NumMemoryIndexes());

		//After the wrap: nothing holds them.
		pB = NULL;
		AssertSize(uiRootOnly, gcObjects.NumMemoryIndexes());

		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStack(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestDistToStackSimpleTwoStep();
	TestDistToStackSimpleOneStep();
	TestDistToStackSimpleHeap();
	TestDistToStackNoStackHeapFromBroken();
	TestDistToStackNoHeapA();
	TestDistToStackNoHeapB();
	TestDistToStackNoHeapC();
	TestDistToStackRootCyclicWithStackPointerA();
	TestDistToStackRootCyclicWithStackPointerB();
	TestDistToStackRootCyclicWithStackPointerC();
	TestDistToStackRootCyclicWithoutStackPointer();
	TestDistToStackSplitRootAndStack();
	TestDistToStackSetWithoutStackPointers();
	TestDistToStackSetWithStackPointers();
	TestDistToStackCyclicWithScenarioA();
	TestDistToStackSetBroken();
	TestDistToStackRemoveUnbalancedLargeBroken();
	TestDistToStackReachedThroughEmbedded();
	TestDistToStackIsHeldFromStackOrRoot();
	TestDistToStackAcrossWalkStampWrap();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

