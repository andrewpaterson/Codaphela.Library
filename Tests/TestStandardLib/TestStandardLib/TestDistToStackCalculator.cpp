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
	AssertInt(UNKNOWN_DIST_TO_STACK, p1.Object()->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, p2.Object()->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, p3.Object()->GetDistToStack());

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

	CDistCalculatorParameters	cParameters;
	CDistToStackCalculator		cDistToStackCalculator;

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

	AssertInt(UNKNOWN_DIST_TO_STACK, pc2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pc3->GetDistToStack());

	p2.BaseObject()->TestRemoveStackFrom(p2.This());
	p2.UnsafeClearObject();

	AssertSize(1, pc2->NumHeapFroms());
	AssertSize(0, pc2->NumStackFroms());
	AssertSize(1, p1->NumStackFroms());
	AssertSize(1, p3->NumStackFroms());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pc2, &cParameters);

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

	AssertInt(2, pc2->GetDistToStack());
	AssertInt(1, pc3->GetDistToStack());

	cParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pc1, &cDistParameters);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);

	AssertSize(3, cDistParameters.NumDetachedFromRoot());
	AssertSize(0, cDistParameters.NumCompletelyDetached());

	cDistParameters.ClearTouchedFlags();
	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pc1, &cDistParameters);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());

	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(3, cDistParameters.NumDetachedFromRoot());
	AssertSize(0, cDistParameters.NumCompletelyDetached());

	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pc1, &cDistParameters);
	
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pStack->GetDistToRoot());
	
	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(3, cDistParameters.NumDetachedFromRoot());
	AssertSize(0, cDistParameters.NumCompletelyDetached());

	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pc1, &cDistParameters);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pc1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pc3->GetDistToRoot());
	
	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(3, cDistParameters.NumDetachedFromRoot());
	AssertSize(3, cDistParameters.NumCompletelyDetached());
	AssertPointer(pc1, cDistParameters.GetCompletelyDetached(0));
	AssertPointer(pc3, cDistParameters.GetCompletelyDetached(1));
	AssertPointer(pc2, cDistParameters.GetCompletelyDetached(2));

	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pcTestC, &cDistParameters);

	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestC->GetDistToRoot());
	AssertInt(3, pcTestL1->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestR1->GetDistToRoot());
	AssertInt(4, pcTestL2->GetDistToRoot());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pcTestR2->GetDistToRoot());

	AssertSize(3, cDistParameters.NumTouched());

	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(3, cDistParameters.NumDetachedFromRoot());
	AssertSize(1, cDistParameters.NumCompletelyDetached());
	AssertPointer(pcTestC, cDistParameters.GetCompletelyDetached(0));


	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pcSet, &cDistParameters);
	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(2, cDistParameters.NumDetachedFromRoot());
	AssertSize(2, cDistParameters.NumCompletelyDetached());
	AssertPointer(pcSet, cDistParameters.GetCompletelyDetached(0));
	AssertPointer(pcTest, cDistParameters.GetCompletelyDetached(1));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistParameters.GetCompletelyDetachedArray());
	AssertLong(3, gcObjects.NumMemoryIndexes());

	cDistParameters.Kill();

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

	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;
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

	cDistParameters.Init();
	
	cDistToRootCalculator.Calculate(pcSet, &cDistParameters);
	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(2, cDistParameters.NumDetachedFromRoot());
	AssertSize(1, cDistParameters.NumCompletelyDetached());
	AssertPointer(pcSet, cDistParameters.GetCompletelyDetached(0));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistParameters.GetCompletelyDetachedArray());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	cDistParameters.Kill();

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
	CDistToRootCalculator		cDistToRootCalculator;
	CDistToStackCalculator		cDistToStackCalculator;
	CDistCalculatorParameters	cDistParameters;

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

	cDistParameters.Init();
	cDistToRootCalculator.Calculate(pcTest3, &cDistParameters);
	AssertInt(3, pcTest2->GetDistToRoot());

	cDistToStackCalculator.CalculateFromTouched(&cDistParameters);
	cDistParameters.ClearTouchedFlags();

	AssertSize(1, cDistParameters.NumDetachedFromRoot());
	AssertSize(1, cDistParameters.NumCompletelyDetached());
	AssertPointer(pcTest3, cDistParameters.GetCompletelyDetached(0));

	AssertLong(5, gcObjects.NumMemoryIndexes());
	gcObjects.Remove(cDistParameters.GetCompletelyDetachedArray());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	cDistParameters.Kill();

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

	CDistCalculatorParameters	cParameters;
	CDistToStackCalculator		cDistToStackCalculator;

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
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pcTest1, &cParameters);
	
	AssertInt(1, pcTest1->GetDistToStack());
	AssertInt(2, pcTest2->GetDistToStack());
	AssertInt(2, pcTest3->GetDistToStack());
	AssertInt(3, pcTest4->GetDistToStack());
	AssertInt(3, pcTest5->GetDistToStack());

	cDistToStackCalculator.ResetObjectsToUnknownDistToStack(&cParameters);
	cParameters.Kill();

	// Test from pTest2
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pcTest2, &cParameters);

	AssertInt(1, pcTest1->GetDistToStack());
	AssertInt(2, pcTest2->GetDistToStack());
	AssertInt(2, pcTest3->GetDistToStack());
	AssertInt(3, pcTest4->GetDistToStack());
	AssertInt(3, pcTest5->GetDistToStack());

	cDistToStackCalculator.ResetObjectsToUnknownDistToStack(&cParameters);
	cParameters.Kill();

	// Test from pTest3
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pcTest3, &cParameters);

	AssertInt(1, pcTest1->GetDistToStack());
	AssertInt(2, pcTest2->GetDistToStack());
	AssertInt(2, pcTest3->GetDistToStack());
	AssertInt(3, pcTest4->GetDistToStack());
	AssertInt(3, pcTest5->GetDistToStack());

	cDistToStackCalculator.ResetObjectsToUnknownDistToStack(&cParameters);
	cParameters.Kill();

	// Test from pTest4
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pcTest4, &cParameters);

	AssertInt(1, pcTest1->GetDistToStack());
	AssertInt(2, pcTest2->GetDistToStack());
	AssertInt(2, pcTest3->GetDistToStack());
	AssertInt(3, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());  //Removing a stack pointer to 4 should be unable to effect 5 in any way.

	cDistToStackCalculator.ResetObjectsToUnknownDistToStack(&cParameters);
	cParameters.Kill();

	// Test from pTest5
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest1->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

	cParameters.Init();
	cDistToStackCalculator.Calculate(pcTest5, &cParameters);

	AssertInt(1, pcTest1->GetDistToStack());
	AssertInt(2, pcTest2->GetDistToStack());
	AssertInt(2, pcTest3->GetDistToStack());
	AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());  //Removing a stack pointer to 5 should be unable to effect 4 in any way.
	AssertInt(3, pcTest5->GetDistToStack());

	cDistToStackCalculator.ResetObjectsToUnknownDistToStack(&cParameters);
	cParameters.Kill();

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
		CDistCalculatorParameters	cParameters;
		CDistToStackCalculator		cDistToStackCalculator;

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

		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

		pSet->Remove(pcTest2);

		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest2->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest3->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest4->GetDistToStack());
		AssertInt(UNKNOWN_DIST_TO_STACK, pcTest5->GetDistToStack());

		gcObjects.ValidateObjectsConsistency();

		pcTest2->TestRemoveStackFrom(pTest2.This());
		pTest2.UnsafeClearObject();

		cParameters.Init();
		cDistToStackCalculator.Calculate(pcTest2, &cParameters);

		papcKilled = cParameters.GetCompletelyDetachedArray();
		AssertSize(4, papcKilled->NumElements());

		cParameters.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDistToStackClosestHeapFromToStackEmbedded(void)
{
	ObjectsInit();
	{
		Ptr<CEmbeddedContainer>	pContainer;
		Ptr<CPointerContainer>	pFar;
		Ptr<CPointerContainer>	pNear;
		CBaseObject*			pcFar;
		CBaseObject*			pcNear;
		CBaseObject*			pcClosest;

		pContainer = OMalloc<CEmbeddedContainer>();
		pFar = OMalloc<CPointerContainer>();
		pNear = OMalloc<CPointerContainer>();

		//
		//  Far[3]  Near[1]
		//    |       |
		//    |     mcOne
		//    |       |
		//   Container
		//

		pFar->mp = pContainer;
		pNear->mp = pContainer->GetOne();
		AssertSize(2, pContainer->NumHeapFroms());
		AssertSize(1, pContainer->mcOne.NumHeapFroms());

		pcFar = &pFar;
		pcNear = &pNear;
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcFar->GetDistToRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcNear->GetDistToRoot());

		pcFar->SetDistToStack(3);
		pcNear->SetDistToStack(1);

		//The from to the embedded object is closer to the stack than the from to the container.
		pcClosest = pContainer->GetClosestHeapFromToStack();
		AssertPointer(pcNear, pcClosest);

		pcFar->SetDistToStack(UNKNOWN_DIST_TO_STACK);
		pcNear->SetDistToStack(UNKNOWN_DIST_TO_STACK);

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
	TestDistToStackClosestHeapFromToStackEmbedded();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

