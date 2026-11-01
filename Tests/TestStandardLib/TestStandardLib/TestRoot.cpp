#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/Root.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootDistance(void)
{
	STestObjectFreedNotifier	sKilled1;  //pTest1 is only freed by ObjectsKill so the notifier must outlive it.

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		int							iDist;
		CEmbeddedObject* pcFrom;

		pRoot = ORoot();
		AssertInt(ROOT_DIST_TO_ROOT, pRoot->GetDistToRoot());
		AssertInt(ROOT_DIST_TO_ROOT + 1, pRoot->TestGetSet()->GetDistToRoot());

		pTest1 = OMalloc<CTestObject>(&sKilled1);
		AssertInt(UNATTACHED_DIST_TO_ROOT, pTest1->GetDistToRoot());

		pRoot->Add(pTest1);
		AssertSize(1, pTest1->NumHeapFroms());
		pcFrom = pTest1->GetHeapFrom(0);  //RootSet.
		pcFrom = pcFrom->GetHeapFrom(0);  //Root

		AssertTrue(pcFrom->IsRoot());
		iDist = pTest1->GetDistToRoot();
		AssertInt(2, iDist);
		pcFrom = pTest1->GetHeapFrom(0);  //RootSet.
		AssertInt(1, pcFrom->GetDistToRoot());
		pcFrom = pcFrom->GetHeapFrom(0);  //Root
		AssertInt(0, pcFrom->GetDistToRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootStackPointers(void)
{
	STestObjectFreedNotifier	sKilled1;
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		size						iFroms;

		CObject* pcTest1;
		CObject* pcTest3;

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
		//        pTest2[5]
		//         ^ ^
		//        /   \
		//       /     \
		//      /       \
		// pTest1[1]   pTest3[2]
		//      ^       ^
		//       \     /
		//        \   /
		//         \ /
		//         ...
		//        pRoot[3] 
		//  

		AssertSize(0, pRoot->NumHeapFroms());
		AssertSize(1, pRoot->NumPointerTos());

		iFroms = pTest2->NumHeapFroms();
		AssertSize(2, iFroms);
		pcTest1 = (CObject*)pTest2->GetHeapFrom(0);
		AssertPointer(&pTest1, pcTest1);
		pcTest3 = (CObject*)pTest2->GetHeapFrom(1);
		AssertPointer(&pTest3, pcTest3);
		AssertSize(0, pTest2->NumPointerTos());
		AssertInt(3, pTest2->GetDistToRoot());

		iFroms = pTest1->NumHeapFroms();
		AssertSize(1, iFroms);

		iFroms = pTest3->NumHeapFroms();
		AssertSize(1, iFroms);

		pcTest3 = &pTest3;

		iFroms = pTest3->NumHeapFroms();
		AssertSize(1, iFroms);
		AssertLong(5, gcObjects.NumMemoryIndexes());

		//Stop the stack from pointing.
		pTest1 = NULL;
		pTest2 = NULL;
		pTest3 = NULL;

		AssertFalse(sKilled1.bFreed);
		AssertFalse(sKilled3.bFreed);
		AssertFalse(sKilled2.bFreed);
		AssertLong(5, gcObjects.NumMemoryIndexes());

		pRoot->Remove(pcTest3);

		AssertFalse(sKilled1.bFreed);
		AssertTrue(sKilled3.bFreed);
		AssertFalse(sKilled2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveSimple(void)
{
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;
	STestObjectFreedNotifier	sKilled4;
	STestObjectFreedNotifier	sKilled5;
	STestObjectFreedNotifier	sKilledTop1;
	STestObjectFreedNotifier	sKilledTop2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CSetObject>				pSet;
		Ptr<CTestObject>			pTop1;
		Ptr<CTestObject>			pTop2;

		pRoot = ORoot();

		pTest2 = OMalloc<CTestObject>(&sKilled2);
		pTest3 = OMalloc<CTestObject>(&sKilled3);
		pTest4 = OMalloc<CTestObject>(&sKilled4);
		pTest5 = OMalloc<CTestObject>(&sKilled5);
		pSet = OMalloc<CSetObject>(false);
		pTop1 = OMalloc<CTestObject>(&sKilledTop1);
		pTop2 = OMalloc<CTestObject>(&sKilledTop2);


		pSet->Add(pTop1);
		AssertInt(-1, pSet->GetDistToRoot());  //Not yet root aware.
		AssertInt(-1, pTop1->GetDistToRoot());
		AssertSize(1, pSet->NumPointerTos());
		AssertSize(0, pTop1->NumPointerTos());
		AssertSize(0, pSet->NumHeapFroms());
		AssertSize(1, pTop1->NumHeapFroms());

		// Top1
		//  |
		// Set


		pTest2->mpTest = pTest3;
		pTest4->mpTest = pTest5;
		AssertInt(-1, pTest2->GetDistToRoot());  //Not yet root aware.
		AssertInt(-1, pTest3->GetDistToRoot());
		AssertSize(1, pTest2->NumPointerTos());
		AssertSize(0, pTest3->NumPointerTos());
		AssertSize(0, pTest2->NumHeapFroms());
		AssertSize(1, pTest3->NumHeapFroms());

		// Top1    Test3    Test5
		//  |        |	      |
		// Set     Test2    Test4


		pRoot->Add(pTest2);
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertSize(1, pTest2->NumPointerTos());
		AssertSize(0, pTest3->NumPointerTos());
		AssertSize(1, pTest2->NumHeapFroms());
		AssertSize(1, pTest3->NumHeapFroms());

		//  Top1    Test3    Test5
		//   |        |	      |
		//  Set     Test2    Test4
		//            |
		//          Root


		pTest3->mpTest = pTest4;
		AssertInt(4, pTest4->GetDistToRoot());
		AssertInt(5, pTest5->GetDistToRoot());

		//          Test5   
		//            |   
		//          Test4   
		//            |   
		//  Top1    Test3
		//   |        |	
		//  Set     Test2
		//            |
		//          Root


		pTest5->mpObject = pSet;
		AssertInt(6, pSet->GetDistToRoot());
		AssertInt(7, pTop1->GetDistToRoot());
		pSet->Add(pTop2);
		AssertInt(7, pTop2->GetDistToRoot());
		AssertSize(1, pSet->NumHeapFroms());
		AssertSize(2, pSet->NumPointerTos());

		//   Top1  Top2
		//     \   /
		//      Set 
		//       |
		//     Test5
		//       |  
		//     Test4
		//       |  
		//     Test3
		//       |	
		//     Test2
		//       |
		//     Root


		pTest2->mpObject = pTest4;
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(5, pSet->GetDistToRoot());
		AssertInt(6, pTop1->GetDistToRoot());
		AssertInt(6, pTop2->GetDistToRoot());

		//   Top1  Top2
		//     \   /
		//      Set 
		//       |
		//     Test5
		//       |  
		//     Test4
		//     |   \  
		//     |  Test3
		//      \  /	
		//     Test2
		//       |
		//     Root


		pTest3->mpObject = pTop2;
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(5, pSet->GetDistToRoot());
		AssertInt(6, pTop1->GetDistToRoot());
		AssertInt(4, pTop2->GetDistToRoot());

		//   Top1(6)  Top2(4)
		//     \     / \
		//      \   /   \
		//      Set(5)   |
		//       |       |
		//     Test5(4)  |
		//       |       |
		//     Test4(3)  |
		//     |    \    |
		//     |   Test3(3)
		//      \   /	
		//     Test2(2)
		//        |
		//       ...
		//      Root(0)


		AssertFalse(sKilled2.bFreed);
		AssertFalse(sKilled3.bFreed);
		AssertFalse(sKilled4.bFreed);
		AssertFalse(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);

		AssertSize(2, pTest2->NumPointerTos());
		AssertPointer(&pTest4, pTest2->TestGetPointerTo(0));
		AssertPointer(&pTest3, pTest2->TestGetPointerTo(1));
		AssertPointer(&pTest4, &pTest2->mpObject);
		AssertPointer(&pTest3, &pTest2->mpTest);
		AssertSize(1, pTest2->NumHeapFroms());
		AssertPointer(pRoot->TestGetSet(), pTest2->GetHeapFrom(0));

		pTest3 = NULL;
		pTest2->mpTest = NULL;
		AssertFalse(sKilled2.bFreed);
		AssertTrue(sKilled3.bFreed);
		AssertFalse(sKilled4.bFreed);
		AssertFalse(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(5, pSet->GetDistToRoot());
		AssertInt(6, pTop1->GetDistToRoot());
		AssertInt(6, pTop2->GetDistToRoot());

		AssertSize(1, pTest2->NumPointerTos());
		AssertPointer(&pTest4, pTest2->TestGetPointerTo(0));
		AssertPointer(&pTest4, &pTest2->mpObject);
		AssertSize(1, pTest2->NumHeapFroms());
		AssertPointer(pRoot->TestGetSet(), pTest2->GetHeapFrom(0));

		//   Top1(6)  Top2(6)
		//     \     / 
		//      \   /  
		//      Set(5) 
		//       |     
		//     Test5(4)
		//       |     
		//     Test4(3)
		//       |
		//     Test2(2)
		//       |
		//      ...
		//     Root(0)


		pTest2->mpTest = pTest4;
		pTest2->mpObject = pSet;
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(3, pSet->GetDistToRoot());
		AssertInt(4, pTop1->GetDistToRoot());
		AssertInt(4, pTop2->GetDistToRoot());

		//      Top1(4)  Top2(4)
		//        \     / 
		//         \   /  
		//         Set(3) 
		//        /    \
		//     Test5(4) |
		//       |      |
		//     Test4(3) |
		//       |      |
		//       |     /
		//        \   /
		//       Test2(2)
		//          |
		//         ...
		//        Root(0)

		pTest5 = NULL;
		pTest4 = NULL;
		pTest2->mpTest = NULL;
		AssertFalse(sKilled2.bFreed);
		AssertTrue(sKilled4.bFreed);
		AssertTrue(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertInt(2, pTest2->GetDistToRoot());
		AssertInt(3, pSet->GetDistToRoot());
		AssertInt(4, pTop1->GetDistToRoot());
		AssertInt(4, pTop2->GetDistToRoot());

		//  Top1(4)  Top2(4)
		//      \     / 
		//       \   /  
		//       Set(3) 
		//        |
		//     Test2(2)
		//        |
		//       ...
		//      Root(0)


		pTop1->mpTest = pTop2;
		pTop2->mpTest = pTop1;
		AssertInt(4, pTop1->GetDistToRoot());
		AssertInt(4, pTop2->GetDistToRoot());

		pTop1 = NULL;
		pTop2 = NULL;
		pSet = NULL;

		pTest2->mpObject = NULL;

		AssertFalse(sKilled2.bFreed);
		AssertTrue(sKilledTop1.bFreed);
		AssertTrue(sKilledTop2.bFreed);
		AssertInt(2, pTest2->GetDistToRoot());

		//   Test2(2)
		//      |
		//     ...
		//    Root(0)

		pTest2 = NULL;

		pRoot->RemoveAll();
		AssertTrue(sKilled2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveComplex(void)
{
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;
	STestObjectFreedNotifier	sKilled4;
	STestObjectFreedNotifier	sKilled5;
	STestObjectFreedNotifier	sKilledTop1;
	STestObjectFreedNotifier	sKilledTop2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CSetObject>				pSet;
		CSetObject* pcSet;
		Ptr<CTestObject>			pTop1;
		Ptr<CTestObject>			pTop2;

		pRoot = ORoot();

		pTest2 = OMalloc<CTestObject>(&sKilled2);
		pTest3 = OMalloc<CTestObject>(&sKilled3);
		pTest4 = OMalloc<CTestObject>(&sKilled4);
		pTest5 = OMalloc<CTestObject>(&sKilled5);
		pSet = OMalloc<CSetObject>(false);
		pTop1 = OMalloc<CTestObject>(&sKilledTop1);
		pTop2 = OMalloc<CTestObject>(&sKilledTop2);

		pRoot->Add(pSet);
		pSet->Add(pTest4);
		pSet->Add(pTest2);
		pTest4->mpTest = pTest5;
		pTest2->mpTest = pTest3;
		pTop1->mpTest = pTop2;
		pTop2->mpTest = pTop1;
		pTest5->mpTest = pTop1;
		pTest3->mpTest = pTop2;

		AssertSize(2, pTop1->NumHeapFroms());
		AssertPointer(&pTop2, pTop1->GetHeapFrom(0));
		AssertPointer(&pTest5, pTop1->GetHeapFrom(1));
		AssertSize(2, pTop2->NumHeapFroms());
		AssertPointer(&pTop1, pTop2->GetHeapFrom(0));
		AssertPointer(&pTest3, pTop2->GetHeapFrom(1));
		AssertSize(2, pSet->NumPointerTos());
		AssertPointer(&pTest4, pSet->TestGetPointerTo(0));
		AssertPointer(&pTest2, pSet->TestGetPointerTo(1));
		AssertSize(1, pTest4->NumPointerTos());
		AssertPointer(&pTest5, pTest4->TestGetPointerTo(0));
		AssertSize(1, pTest4->NumHeapFroms());
		AssertPointer(&pSet, pTest4->GetHeapFrom(0));
		AssertInt(5, pTop1->GetDistToRoot());
		AssertInt(5, pTop2->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(4, pTest3->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(2, pSet->GetDistToRoot());

		//   Top1(5)====Top2(5)
		//     |         |
		//   Test5(4)  Test3(4)
		//	   |         |	
		//	 Test4(3)  Test2(3)
		//      \       /
		//       \     /
		//        \   /
		//        Set(2)
		//          |
		//         ...
		//        Root(0)

		pTest5 = NULL;

		pTest4->mpTest = NULL;
		AssertFalse(sKilled2.bFreed);
		AssertFalse(sKilled3.bFreed);
		AssertFalse(sKilled4.bFreed);
		AssertTrue(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertSize(1, pTop1->NumHeapFroms());
		AssertPointer(&pTop2, pTop1->GetHeapFrom(0));
		AssertSize(2, pTop2->NumHeapFroms());
		AssertPointer(&pTop1, pTop2->GetHeapFrom(0));
		AssertPointer(&pTest3, pTop2->GetHeapFrom(1));
		AssertSize(2, pSet->NumPointerTos());
		AssertPointer(&pTest4, pSet->TestGetPointerTo(0));
		AssertPointer(&pTest2, pSet->TestGetPointerTo(1));
		AssertSize(0, pTest4->NumPointerTos());
		AssertPointer(NULL, pTest4->TestGetPointerTo(0));
		AssertSize(1, pTest4->NumHeapFroms());
		AssertPointer(&pSet, pTest4->GetHeapFrom(0));

		//Tested for root should never be true (outside of the CanFindRoot method).
		AssertFalse(pSet->TestedForRoot());
		AssertFalse(pTest2->TestedForRoot());
		AssertFalse(pTest3->TestedForRoot());
		AssertFalse(pTest4->TestedForRoot());
		AssertFalse(pTop1->TestedForRoot());
		AssertFalse(pTop2->TestedForRoot());

		//   Top1====Top2
		//            |
		//          Test3
		//	          |	
		//	 Test4  Test2
		//      \    /
		//       \  /
		//       Set
		//        |
		//       ...
		//      Root(0)


		pTest5 = OMalloc<CTestObject>(&sKilled5);
		pTest4->mpTest = pTest5;
		pTest5->mpTest = pTop1;
		AssertSize(2, pTop1->NumHeapFroms());
		AssertPointer(&pTop2, pTop1->GetHeapFrom(0));
		AssertPointer(&pTest5, pTop1->GetHeapFrom(1));
		AssertSize(2, pTop2->NumHeapFroms());
		AssertPointer(&pTop1, pTop2->GetHeapFrom(0));
		AssertPointer(&pTest3, pTop2->GetHeapFrom(1));
		AssertSize(2, pSet->NumPointerTos());
		AssertPointer(&pTest4, pSet->TestGetPointerTo(0));
		AssertPointer(&pTest2, pSet->TestGetPointerTo(1));
		AssertSize(1, pTest4->NumPointerTos());
		AssertPointer(&pTest5, pTest4->TestGetPointerTo(0));
		AssertSize(1, pTest4->NumHeapFroms());
		AssertPointer(&pSet, pTest4->GetHeapFrom(0));
		AssertInt(5, pTop1->GetDistToRoot());
		AssertInt(5, pTop2->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(4, pTest3->GetDistToRoot());
		AssertInt(3, pTest4->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(2, pSet->GetDistToRoot());
		AssertFalse(pSet->TestedForRoot());
		AssertFalse(pTest2->TestedForRoot());
		AssertFalse(pTest3->TestedForRoot());
		AssertFalse(pTest4->TestedForRoot());
		AssertFalse(pTest5->TestedForRoot());
		AssertFalse(pTop1->TestedForRoot());
		AssertFalse(pTop2->TestedForRoot());

		//   Top1====Top2
		//     |      |
		//   Test5  Test3
		//	   |      |	
		//	 Test4  Test2
		//      \    /
		//       \  /
		//       Set
		//        |
		//       ...
		//      Root(0)

		pTest2 = NULL;
		pTest3 = NULL;
		pTest5 = NULL;
		pTest4 = NULL;
		pTop1 = NULL;
		pTop2 = NULL;
		pcSet = &pSet;
		pSet = NULL;

		pRoot->Remove(pcSet);
		AssertTrue(sKilled2.bFreed);
		AssertTrue(sKilled3.bFreed);
		AssertTrue(sKilled4.bFreed);
		AssertTrue(sKilled5.bFreed);
		AssertTrue(sKilledTop1.bFreed);
		AssertTrue(sKilledTop2.bFreed);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveUnbalancedLarge(void)
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

		//       Test1(4)  
		//       //\\      
		//      //  \\       
		//   Top1(5) Top2(3)
		//     |      |
		//   Test5(6) |
		//     |      |
		//   Test4(5) |
		//     |      |
		//   Test3(4) |
		//     |	  |
		//   Test2(3) |
		//      \    /
		//       \  /
		//       Set(2)
		//        |
		//       ...
		//      Root(0)

		pTest3 = NULL;
		pTest4 = NULL;
		pTest5 = NULL;

		pcTest2 = &pTest2;
		pTest2 = NULL;

		pSet->Remove(pcTest2);

		AssertInt(2, pSet->GetDistToRoot());
		AssertInt(3, pTop2->GetDistToRoot());
		AssertInt(4, pTest1->GetDistToRoot());
		AssertInt(5, pTop1->GetDistToRoot());
		AssertTrue(sKilled2.bFreed);
		AssertTrue(sKilled3.bFreed);
		AssertTrue(sKilled4.bFreed);
		AssertTrue(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertFalse(sKilled1.bFreed);
		AssertTrue(pSet->CanFindRoot());
		AssertTrue(pTop2->CanFindRoot());
		AssertTrue(pTest1->CanFindRoot());
		AssertTrue(pTop1->CanFindRoot());
		AssertSize(1, pSet->NumElements());
		AssertSize(1, pSet->NumPointerTos());
		AssertSize(1, pTop2->NumPointerTos());
		AssertSize(2, pTest1->NumPointerTos());
		AssertSize(1, pTop1->NumPointerTos());

		//   Top1(5)
		//     ||
		//   Test1(4)  
		//     ||
		//    Top2(3)
		//     |
		//    Set(2)
		//     |
		//    ...
		//   Root(0)
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveUnbalancedSmall(void)
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
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CSetObject>				pSet;
		Ptr<CTestObject>			pTop1;
		Ptr<CTestObject>			pTop2;

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

		//
		//       Test1(4)  
		//       //\\      
		//      //  \\       
		//   Top1(5) Top2(3)
		//     |      |
		//   Test5(6) |
		//     |      |
		//   Test4(5) |
		//     |      |
		//   Test3(4) |
		//     |	  |
		//   Test2(3) |
		//      \    /
		//       \  /
		//       Set(2)
		//        |
		//       ...
		//      Root(0)
		//

		pSet->Remove(pTop2);
		AssertSize(1, pSet->NumElements());
		AssertInt(2, pSet->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(4, pTest3->GetDistToRoot());
		AssertInt(5, pTest4->GetDistToRoot());
		AssertInt(6, pTest5->GetDistToRoot());
		AssertInt(7, pTop1->GetDistToRoot());
		AssertInt(8, pTest1->GetDistToRoot());
		AssertInt(9, pTop2->GetDistToRoot());
		AssertFalse(sKilled2.bFreed);
		AssertFalse(sKilled3.bFreed);
		AssertFalse(sKilled4.bFreed);
		AssertFalse(sKilled5.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertFalse(sKilled1.bFreed);

		//   Top2(9)
		//     |
		//   Test1(8)  
		//     |  
		//   Top1(7)
		//     |    
		//   Test5(6)
		//     |    
		//   Test4(5)
		//     |    
		//   Test3(4)
		//     |	
		//   Test2(3)
		//     |
		//    Set(2)
		//     |
		//    ...
		//   Root(0)
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveMostlyBalanced(void)
{
	STestObjectFreedNotifier	sKilled1;
	STestObjectFreedNotifier	sKilled2;
	STestObjectFreedNotifier	sKilled3;
	STestObjectFreedNotifier	sKilled4;
	STestObjectFreedNotifier	sKilled5;
	STestObjectFreedNotifier	sKilled6;
	STestObjectFreedNotifier	sKilledTop1;
	STestObjectFreedNotifier	sKilledTop2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CTestObject>			pTest6;
		Ptr<CSetObject>				pSet;
		Ptr<CTestObject>			pTop1;
		Ptr<CTestObject>			pTop2;
		CTestObject* pcTest2;

		pRoot = ORoot();

		pTest1 = OMalloc<CTestObject>(&sKilled1);
		pTest2 = OMalloc<CTestObject>(&sKilled2);
		pTest3 = OMalloc<CTestObject>(&sKilled3);
		pTest4 = OMalloc<CTestObject>(&sKilled4);
		pTest5 = OMalloc<CTestObject>(&sKilled5);
		pTest6 = OMalloc<CTestObject>(&sKilled6);
		pSet = OMalloc<CSetObject>(false);
		pTop1 = OMalloc<CTestObject>(&sKilledTop1);
		pTop2 = OMalloc<CTestObject>(&sKilledTop2);

		pRoot->Add(pSet);
		pSet->Add(pTest3);
		pSet->Add(pTest2);

		pTest2->mpTest = pTop2;
		pTest5->mpTest = pTop1;
		pTest4->mpTest = pTest5;
		pTest3->mpTest = pTest4;
		pTop1->mpTest = pTest1;
		pTop2->mpTest = pTest1;
		pTop2->mpObject = pTest6;
		pTest1->mpTest = pTop1;
		pTest1->mpObject = pTop2;
		pTest6->mpTest = pTop2;

		AssertInt(2, pSet->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertInt(4, pTest4->GetDistToRoot());
		AssertInt(5, pTest5->GetDistToRoot());
		AssertInt(6, pTop1->GetDistToRoot());
		AssertInt(5, pTest1->GetDistToRoot());
		AssertInt(4, pTop2->GetDistToRoot());
		AssertSize(2, pSet->NumElements());
		AssertSize(3, pTop2->NumHeapFroms());

		//      Test1(5)  Test6(5)
		//       //  \\    //  
		//      //    \\  //     
		//  Top1(6)   Top2(4)
		//     |        |
		//  Test5(5)    |
		//     |        |
		//  Test4(4)    |
		//     |        |
		//  Test3(3)  Test2(3)
		//      \      /
		//       \    /
		//       Set(2)
		//        |
		//       ...
		//      Root(0)

		pcTest2 = &pTest2;
		pTest2 = NULL;
		pSet->Remove(pcTest2);

		AssertInt(2, pSet->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertInt(4, pTest4->GetDistToRoot());
		AssertInt(5, pTest5->GetDistToRoot());
		AssertInt(6, pTop1->GetDistToRoot());
		AssertInt(7, pTest1->GetDistToRoot());
		AssertInt(8, pTop2->GetDistToRoot());
		AssertInt(9, pTest6->GetDistToRoot());

		AssertTrue(sKilled2.bFreed);
		AssertFalse(sKilled3.bFreed);
		AssertFalse(sKilled4.bFreed);
		AssertFalse(sKilled5.bFreed);
		AssertFalse(sKilled6.bFreed);
		AssertFalse(sKilledTop1.bFreed);
		AssertFalse(sKilledTop2.bFreed);
		AssertFalse(sKilled1.bFreed);
		AssertSize(1, pSet->NumElements());

		//
		//   Test6(9)       
		//     ||     
		//   Top2(8)
		//     ||
		//   Test1(7)  
		//     || 
		//   Top1(6)
		//     |    
		//   Test5(5)
		//     |    
		//   Test4(4)
		//     |    
		//   Test3(3)
		//     |	
		//    Set(2)
		//     |
		//    ...
		//   Root(0)
		//
	}
	ObjectsFlush();
	ObjectsKill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveErrorFromPointerRemapping(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;
	STestObjectFreedNotifier	sFreedNotifier3;
	STestObjectFreedNotifier	sFreedNotifier4;
	STestObjectFreedNotifier	sFreedNotifier5;
	STestObjectFreedNotifier	sFreedNotifier6;
	STestObjectFreedNotifier	sFreedNotifier7;
	STestObjectFreedNotifier	sFreedNotifier8;
	STestObjectFreedNotifier	sFreedNotifier9;
	STestObjectFreedNotifier	sFreedNotifier15;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestObject>			pTest1;
		Ptr<CTestObject>			pTest2;
		Ptr<CTestObject>			pTest3;
		Ptr<CTestObject>			pTest4;
		Ptr<CTestObject>			pTest5;
		Ptr<CTestObject>			pTest6;
		Ptr<CTestObject>			pTest7;
		Ptr<CTestObject>			pTest8;
		Ptr<CTestObject>			pTest9;
		Ptr<CTestObject>			pTest15;

		pRoot = ORoot();

		pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
		pTest2 = OMalloc<CTestObject>(&sFreedNotifier2);
		pTest3 = OMalloc<CTestObject>(&sFreedNotifier3);
		pTest4 = OMalloc<CTestObject>(&sFreedNotifier4);
		pTest5 = OMalloc<CTestObject>(&sFreedNotifier5);
		pTest6 = OMalloc<CTestObject>(&sFreedNotifier6);
		pTest7 = OMalloc<CTestObject>(&sFreedNotifier7);
		pTest8 = OMalloc<CTestObject>(&sFreedNotifier8);
		pTest9 = OMalloc<CTestObject>(&sFreedNotifier9);
		pTest15 = OMalloc<CTestObject>(&sFreedNotifier15);

		pRoot->Add(pTest1);
		pTest1->mpObject = pTest2;
		pTest1->mpTest = pTest3;
		pTest2->mpObject = pTest15;
		pTest15->mpTest = pTest4;
		pTest3->mpObject = pTest5;
		pTest4->mpTest = pTest6;
		pTest5->mpObject = pTest6;
		pTest5->mpTest = pTest7;
		pTest6->mpObject = pTest8;
		pTest6->mpTest = pTest9;
		pTest7->mpTest = pTest9;

		AssertInt(2, pTest1->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(3, pTest3->GetDistToRoot());
		AssertInt(4, pTest15->GetDistToRoot());
		AssertInt(4, pTest5->GetDistToRoot());
		AssertInt(5, pTest4->GetDistToRoot());
		AssertInt(5, pTest6->GetDistToRoot());
		AssertInt(5, pTest7->GetDistToRoot());
		AssertInt(6, pTest8->GetDistToRoot());
		AssertInt(6, pTest9->GetDistToRoot());

		AssertLong(12, gcObjects.NumMemoryIndexes());

		//
		// Test8[10](6) Test9[11](6)
		//	   |       / |
		//	   |      /  |
		//	   |     /   |
		//	   |    /    |
		//	   |   /     |
		// Test6[8](5)  Test7[9](5)
		//	   |   \     |	
		//	   |    \    |	
		//	   |     \   |	
		//	   |      \  |	
		//	   |       \ |	
		// Test4[6](5)  Test5[7](4)
		//	   |         |	
		//	   |         |	
		//	   |         |	
		// Test15[12](4) |	
		//	   |         |	
		//	   |         |	
		//	   |         |	
		// Test2[4](3)  Test3[5](3)
		//      \       /
		//       \     X  <---- Broken here
		//        \   /
		//       Test1[3](2)
		//         |
		//         |
		//        ...
		//      Root(0)
		//

		pTest5 = NULL;
		pTest7 = NULL;
		pTest3 = NULL;
		pTest1->mpTest = NULL;

		AssertTrue(sFreedNotifier3.bFreed);
		AssertTrue(sFreedNotifier5.bFreed);
		AssertTrue(sFreedNotifier7.bFreed);
		AssertFalse(sFreedNotifier9.bFreed);

		AssertFalse(sFreedNotifier1.bFreed);
		AssertFalse(sFreedNotifier2.bFreed);
		AssertFalse(sFreedNotifier4.bFreed);
		AssertFalse(sFreedNotifier6.bFreed);
		AssertFalse(sFreedNotifier8.bFreed);
		AssertFalse(sFreedNotifier15.bFreed);

		AssertSize(1, pTest6->NumHeapFroms());
		AssertPointer(&pTest4, pTest6->GetHeapFrom(0));
		AssertSize(2, pTest6->NumPointerTos());

		AssertInt(2, pTest1->GetDistToRoot());
		AssertInt(3, pTest2->GetDistToRoot());
		AssertInt(4, pTest15->GetDistToRoot());
		AssertInt(5, pTest4->GetDistToRoot());
		AssertInt(6, pTest6->GetDistToRoot());
		AssertInt(7, pTest8->GetDistToRoot());
		AssertInt(7, pTest9->GetDistToRoot());

		// pTest8(7) pTest9(7)
		//	   \       /  
		//      \     /   
		//	     \   /    
		//     pTest6(6)  
		//	       |  
		//	       |  
		//     pTest4(5)  
		//	       |       
		//	       |       	
		//	   pTest15(4)
		//	       |      
		//	       |      
		//     pTest2(3) 
		//         |     
		//         |    
		//         |  
		//     pTest1(2)
		//         |
		//         |
		//        ...
		//      Root(0)

		AssertLong(9, gcObjects.NumMemoryIndexes());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootGraphRemoveErrorFromObjectConverter(void)
{
	ObjectsInit();
	{
		Ptr<CTestNamedString>			cNS1;
		Ptr<CTestNamedString>			cNS2;
		Ptr<CTestNamedString>			cDiamond;
		Ptr<CTestDoubleNamedString>	cDouble;
		Ptr<CString>					sz1;
		Ptr<CString>					sz3;
		Ptr<CString>					sz2;
		Ptr<CString>					sz4;
		Ptr<CRoot>						cRoot;

		cRoot = ORoot();

		sz3 = OMalloc<CString>("End");
		cDiamond = ONMalloc<CTestNamedString>("Diamond End", sz3, Null(), "1234");

		sz1 = OMalloc<CString>("World");
		cNS1 = ONMalloc<CTestNamedString>("NamedString 1", sz1, cDiamond, "Hello");

		sz2 = OMalloc<CString>("World");
		cNS2 = ONMalloc<CTestNamedString>("NamedString 2", sz2, cDiamond, "Hello");


		sz4 = OMalloc<CString>("Start");
		cDouble = ONMalloc<CTestDoubleNamedString>("Double Start", sz4, cNS1, Null());

		cRoot->Add(cDouble);

		AssertInt(2, cDouble->GetDistToRoot());
		AssertInt(3, cNS1->GetDistToRoot());
		AssertInt(4, cDiamond->GetDistToRoot());
		AssertInt(-1, cNS2->GetDistToRoot());

		cDouble->mpSplit2 = cNS2;

		AssertInt(3, cNS2->GetDistToRoot());
		AssertInt(4, cDiamond->GetDistToRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootSetRemoveAll(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;

	ObjectsInit();
	{
		Ptr<CPointerContainer>		pContainer1;
		Ptr<CPointerContainer>		pContainer2;
		Ptr<CTestObject>			pObject;
		Ptr<CRoot>					pRoot;
		CTestObject* pcObject;
		CPointerContainer* pcContainer1;
		CPointerContainer* pcContainer2;

		pObject = OMalloc<CTestObject>(&sFreedNotifier1);
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pRoot = ORoot();
		pRoot->Add(pContainer1);

		pcObject = &pObject;
		pcContainer1 = &pContainer1;
		pcContainer2 = &pContainer2;

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcUnknowns.NumElements());

		//This removes all the immediately held objects (but does not kill them if they are pointed to by something else).
		pRoot->RemoveAll();

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcUnknowns.NumElements());

		AssertInt(UNATTACHED_DIST_TO_ROOT, pcContainer1->GetDistToRoot());
		AssertFalse(pcContainer1->CanFindRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcContainer2->GetDistToRoot());
		AssertFalse(pcContainer2->CanFindRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcObject->GetDistToRoot());
		AssertFalse(pcObject->CanFindRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootSetKillAll(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;

	ObjectsInit();
	{
		Ptr<CPointerContainer>		pContainer1;
		Ptr<CPointerContainer>		pContainer2;
		Ptr<CTestObject>			pObject;
		Ptr<CRoot>					pRoot;
		CTestObject* pcObject;
		CPointerContainer* pcContainer2;
		Ptr<CPointerContainer>		pTemp;

		pObject = OMalloc<CTestObject>(&sFreedNotifier1);
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pRoot = ORoot();
		pRoot->Add(pContainer1);

		pcObject = &pObject;
		pcContainer2 = &pContainer2;

		pTemp = OMalloc<CPointerContainer>(pContainer1);

		AssertLong(6, gcObjects.NumMemoryIndexes());
		AssertLong(6, gcUnknowns.NumElements());

		//This kills all the immediately held objects (setting pointers to null as applicable).
		//Objects further down the tree pointed to by stack pointers will not be killed.
		pRoot->KillAll();

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcUnknowns.NumElements());
		AssertNull(&pTemp->mp);
		AssertNull(&pContainer1);
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcContainer2->GetDistToRoot());
		AssertFalse(pcContainer2->CanFindRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcObject->GetDistToRoot());
		AssertFalse(pcObject->CanFindRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootKillWithStackPointers(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;

	ObjectsInit();
	{
		Ptr<CPointerContainer>		pContainer1;
		Ptr<CPointerContainer>		pContainer2;
		Ptr<CTestObject>			pObject;
		Ptr<CRoot>					pRoot;

		pObject = OMalloc<CTestObject>(&sFreedNotifier1);
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pRoot = ORoot();
		pRoot->Add(pContainer1);

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcUnknowns.NumElements());

		//This is not yet implemented.
		pRoot->Kill();

		//These should be 3.
		AssertNull(&pRoot);
		AssertLong(4, gcObjects.NumMemoryIndexes());
		AssertLong(4, gcUnknowns.NumElements());

		AssertInt(UNATTACHED_DIST_TO_ROOT, pObject->GetDistToRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pContainer1->GetDistToRoot());
		AssertInt(UNATTACHED_DIST_TO_ROOT, pContainer2->GetDistToRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRootKill(void)
{
	STestObjectFreedNotifier	sFreedNotifier1;

	ObjectsInit();
	{
		Ptr<CPointerContainer>		pContainer1;
		Ptr<CPointerContainer>		pContainer2;
		Ptr<CTestObject>			pObject;
		Ptr<CRoot>					pRoot;

		AssertLong(0, gcObjects.NumMemoryIndexes());
		AssertLong(0, gcUnknowns.NumElements());

		pObject = OMalloc<CTestObject>(&sFreedNotifier1);
		pContainer2 = OMalloc<CPointerContainer>(pObject);
		pContainer1 = OMalloc<CPointerContainer>(pContainer2);
		pRoot = ORoot();
		pRoot->Add(pContainer1);

		pObject = NULL;
		pContainer1 = NULL;
		pContainer2 = NULL;

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcUnknowns.NumElements());
		Pass();

		//This is not yet implemented.  Killing the root should cause every object to have it's dist to root set to invalid (including the root).
		pRoot->Kill();

		//These should be zero.
		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcUnknowns.NumElements());
		Pass();

		pRoot = ORoot();
		AssertString(ROOT_NAME, pRoot->GetName());
		AssertLong(4LL, pRoot->GetIndex());
		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcUnknowns.NumElements());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRoot(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestRootKill();
	TestRootKillWithStackPointers();
	TestRootDistance();
	TestRootStackPointers();
	TestRootGraphRemoveSimple();
	TestRootGraphRemoveComplex();
	TestRootGraphRemoveUnbalancedLarge();
	TestRootGraphRemoveUnbalancedSmall();
	TestRootGraphRemoveMostlyBalanced();
	TestRootGraphRemoveErrorFromPointerRemapping();
	TestRootGraphRemoveErrorFromObjectConverter();
	TestRootSetKillAll();
	TestRootSetRemoveAll();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

