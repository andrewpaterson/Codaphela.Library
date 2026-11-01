#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/CodabaseFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "StandardLib/Root.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReplaceAndRemapAddConstructors(void)
{
	gcObjects.AddConstructor<CTestObject>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReplaceNoPointers(void)
{
	STestObjectFreedNotifier	sFreeNotifider1;
	STestObjectFreedNotifier	sFreeNotifider2;
	STestObjectFreedNotifier	sFreeNotifider3;

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;
		Ptr<CTestObject>			pObject3;
		Ptr<CTestObject>			pObject4;
		Ptr<CRoot>					pRoot;
		OIndex						oi1;
		OIndex						oi3;

		TestReplaceAndRemapAddConstructors();
		AssertSize(0, gcUnknowns.NumElements());

		pRoot = ORoot();
		pObject1 = ONMalloc<CTestObject>("Pavel", &sFreeNotifider1);
		pObject2 = ONMalloc<CTestObject>("Meninski", &sFreeNotifider2);
		pObject3 = ONMalloc<CTestObject>("Ned", &sFreeNotifider3);
		pRoot->Add(pObject1);
		pRoot->Add(pObject2);
		oi1 = pObject1->GetIndex();
		oi3 = pObject3->GetIndex();
		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(4, gcObjects.NumMemoryNames());
		AssertFalse(oi1 == oi3);
		AssertString("GraphRoot", pRoot->GetName());
		AssertSize(5, gcUnknowns.NumElements());

		gcObjects.TestRemoveMemoryIdentifiers(&pObject3);
		gcObjects.ReplaceBaseObject(&pObject1, &pObject3);
		AssertLong(4, gcObjects.NumMemoryIndexes());
		AssertLong(3, gcObjects.NumMemoryNames());
		AssertTrue(sFreeNotifider1.bFreed);
		AssertPointer(&pObject3, &pObject1);
		AssertFalse(sFreeNotifider2.bFreed);
		AssertFalse(sFreeNotifider3.bFreed);
		pObject4 = pRoot->GetFirst<CTestObject>();
		AssertPointer(&pObject3, &pObject4);
		AssertSize(4, gcUnknowns.NumElements());

		pObject4 = gcObjects.Get("Pavel");
		AssertNull(&pObject4);
		pObject4 = gcObjects.Get("Ned");
		AssertPointer(&pObject3, &pObject4);
		pObject4 = gcObjects.Get(oi1);
		AssertNull(&pObject4);
		pObject4 = gcObjects.Get(oi3);
		AssertPointer(&pObject3, &pObject4);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReplaceWithHeapPointers(void)
{
	STestObjectFreedNotifier	sFreeNotifider3;
	STestObjectFreedNotifier	sFreeNotifider4;

	ObjectsInit();
	{
		Ptr<CTestSaveableObject2>	pObject1;
		Ptr<CTestSaveableObject1>	pObject2;
		Ptr<CTestObject>			pObject3;
		Ptr<CTestObject>			pObject4;
		Ptr<CTestEmbeddedStrings>	pObject5;
		Ptr<>						pTest;
		Ptr<CRoot>					pRoot;
		OIndex						oi1;
		OIndex						oi2;
		OIndex						oi3;
		OIndex						oi4;
		OIndex						oi5;

		TestReplaceAndRemapAddConstructors();
		AssertSize(0, gcUnknowns.NumElements());

		pRoot = ORoot();
		pObject1 = ONMalloc<CTestSaveableObject2>("Pavel", "Pavel");
		pObject2 = ONMalloc<CTestSaveableObject1>("Meninski");
		pObject3 = ONMalloc<CTestObject>("Ned", &sFreeNotifider3);
		pObject4 = ONMalloc<CTestObject>("Jeff", &sFreeNotifider4);
		pObject5 = OMalloc<CTestEmbeddedStrings>();
		pRoot->Add(pObject1);
		pRoot->Add(pObject4);
		pObject4->mpObject = pObject1;
		pObject1->mp1 = pObject2;
		pObject1->mp2 = pObject3;
		pObject3->mpObject = pObject2;

		//  pObject2(3) <--- pObject3(3)
		//   "Meninski"      "Ned"
		//        ^          ^
		//         \        /
		//          \      /
		//           \    / 
		//            \  /
		//         pObject1(2) <--- pObject4(2)  
		//             "Pavel"      "Jeff"
		//                ^          ^
		//                 \        /
		//                  \      /
		//                   \    / 
		//                    \  /
		//                    ...
		//                   Root(0)

		oi1 = pObject1->GetIndex();
		oi2 = pObject2->GetIndex();
		oi3 = pObject3->GetIndex();
		oi4 = pObject4->GetIndex();
		oi5 = pObject5->GetIndex();
		pObject2 = NULL;
		pObject3 = NULL;
		pRoot = NULL;
		AssertLong(7, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcObjects.NumMemoryNames());
		AssertSize(7, gcUnknowns.NumElements());
		AssertFalse(oi1 == oi2);
		AssertFalse(oi1 == oi3);
		AssertFalse(oi1 == oi4);
		AssertFalse(oi1 == oi5);
		AssertFalse(sFreeNotifider3.bFreed);
		AssertFalse(sFreeNotifider4.bFreed);

		gcObjects.TestRemoveMemoryIdentifiers(&pObject5);
		gcObjects.ReplaceBaseObject(&pObject1, &pObject5);

		//         pObject5(2) <--- pObject4(2)  
		//               ^          "Jeff"
		//                \          ^
		//                 \        /
		//                  \      /
		//                   \    / 
		//                    \  /
		//                    ...
		//                   Root(0)

		AssertLong(4, gcObjects.NumMemoryIndexes());
		AssertLong(2, gcObjects.NumMemoryNames());
		AssertSize(4, gcUnknowns.NumElements());
		AssertTrue(sFreeNotifider3.bFreed);
		AssertFalse(sFreeNotifider4.bFreed);

		pTest = gcObjects.Get("Pavel");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Meninski");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Ned");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Jeff");
		AssertPointer(&pObject4, &pTest);
		pTest = gcObjects.Get(oi1);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi2);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi3);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi4);
		AssertPointer(&pObject4, &pTest);
		pTest = gcObjects.Get(oi5);
		AssertPointer(&pObject5, &pTest);
		AssertInt(2, pObject5.GetDistToRoot());
		AssertPointer(&pObject5, &pObject1);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectReplaceWithStackPointers(void)
{
	STestObjectFreedNotifier	sFreeNotifider3;
	STestObjectFreedNotifier	sFreeNotifider4;

	ObjectsInit();
	{
		Ptr<CTestSaveableObject2>	pObject1;
		Ptr<CTestSaveableObject1>	pObject2;
		Ptr<CTestObject>			pObject3;
		Ptr<CTestObject>			pObject4;
		Ptr<CTestEmbeddedStrings>	pObject5;
		Ptr<>						pTest;
		Ptr<CRoot>					pRoot;
		OIndex						oi1;
		OIndex						oi2;
		OIndex						oi3;
		OIndex						oi4;
		OIndex						oi5;

		TestReplaceAndRemapAddConstructors();
		AssertSize(0, gcUnknowns.NumElements());

		pRoot = ORoot();
		pObject1 = ONMalloc<CTestSaveableObject2>("Pavel", "Pavel");
		pObject2 = ONMalloc<CTestSaveableObject1>("Meninski");
		pObject3 = ONMalloc<CTestObject>("Ned", &sFreeNotifider3);
		pObject4 = ONMalloc<CTestObject>("Jeff", &sFreeNotifider4);
		pObject5 = OMalloc<CTestEmbeddedStrings>();
		pRoot->Add(pObject1);
		pRoot->Add(pObject4);
		pObject4->mpObject = pObject1;
		pObject1->mp1 = pObject2;
		pObject1->mp2 = pObject3;
		pObject3->mpObject = pObject2;

		//  pObject2(3) <--- pObject3(3)
		//   "Meninski"      "Ned"
		//      ^ ^          ^
		//     .   \        /
		//    .     \      /
		//   .       \    / 
		//            \  /
		//         pObject1(2) <--- pObject4(2)  
		//             "Pavel"      "Jeff"
		//              ^ ^          ^
		//             .   \        /
		//            .     \      /
		//           .       \    / 
		//                    \  /
		//                    ...
		//                   Root(0)

		oi1 = pObject1->GetIndex();
		oi2 = pObject2->GetIndex();
		oi3 = pObject3->GetIndex();
		oi4 = pObject4->GetIndex();
		oi5 = pObject5->GetIndex();
		pObject2 = NULL;
		pObject3 = NULL;
		pRoot = NULL;
		AssertLong(7, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcObjects.NumMemoryNames());
		AssertSize(7, gcUnknowns.NumElements());
		AssertFalse(oi1 == oi2);
		AssertFalse(oi1 == oi3);
		AssertFalse(oi1 == oi4);
		AssertFalse(oi1 == oi5);
		AssertFalse(sFreeNotifider3.bFreed);
		AssertFalse(sFreeNotifider4.bFreed);

		gcObjects.TestRemoveMemoryIdentifiers(&pObject5);
		gcObjects.ReplaceBaseObject(&pObject1, &pObject5);

		//         pObject5(2) <--- pObject4(2)  
		//               ^          "Jeff"
		//                \          ^
		//                 \        /
		//                  \      /
		//                   \    / 
		//                    \  /
		//                    ...
		//                   Root(0)

		AssertLong(4, gcObjects.NumMemoryIndexes());
		AssertLong(2, gcObjects.NumMemoryNames());
		AssertSize(4, gcUnknowns.NumElements());
		AssertTrue(sFreeNotifider3.bFreed);
		AssertFalse(sFreeNotifider4.bFreed);

		pTest = gcObjects.Get("Pavel");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Meninski");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Ned");
		AssertNull(&pTest);
		pTest = gcObjects.Get("Jeff");
		AssertPointer(&pObject4, &pTest);
		pTest = gcObjects.Get(oi1);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi2);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi3);
		AssertNull(&pTest);
		pTest = gcObjects.Get(oi4);
		AssertPointer(&pObject4, &pTest);
		pTest = gcObjects.Get(oi5);
		AssertPointer(&pObject5, &pTest);
		AssertInt(2, pObject5.GetDistToRoot());
		AssertPointer(&pObject5, &pObject1);
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestReplaceAndRemap(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectReplaceNoPointers();
	TestObjectReplaceWithHeapPointers();
	TestObjectReplaceWithStackPointers();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

