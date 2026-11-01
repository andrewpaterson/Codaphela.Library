#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/LogToMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "NamedObjectTestClasses.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorAddConstructors(void)
{
	gcObjects.AddConstructor<CTestNamedObject>();
	gcObjects.AddConstructor<CTestNamedObjectSmall>();
	gcObjects.AddConstructor<CTestNamedObjectWithEmbedded>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorSimpleAdd(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CTestNamedObject>	pNamed;
		Ptr<CTestNamedObject>	pTemp;

		TestObjectAllocatorAddConstructors();

		pNamed = gcObjects.AllocateUninitialisedByClassNameAndAddIntoMemory("CTestNamedObject");
		AssertNotNull(pNamed.Object());

		AssertLong(1LL, pNamed.GetIndex());
		AssertString("", pNamed.GetName());

		pTemp = gcObjects.Get("");
		AssertNull(pTemp.Object());
		pTemp = gcObjects.Get(1LL);
		AssertPointer(pNamed.Object(), pTemp.Object());
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorNamedAdd(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CTestNamedObject>	pNamed1;
		Ptr<CTestNamedObject>	pNamed2;
		Ptr<CTestNamedObject>	pTemp;
		SLogConfig				sLogConfig;

		TestObjectAllocatorAddConstructors();
		AssertLong(0, gcObjects.NumMemoryIndexes());

		pNamed1 = gcObjects.AllocateNamedUninitialisedByClassNameAndAddIntoMemory("CTestNamedObject", "My Object Name");
		AssertNotNull(pNamed1.Object());
		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertFalse(pNamed1->IsInitialised());
		pNamed1->Init(98);
		AssertTrue(pNamed1->IsInitialised());

		AssertLong(1LL, pNamed1.GetIndex());
		AssertString("My Object Name", pNamed1.GetName());
		AssertInt((int32)98, pNamed1->miNum);

		pTemp = gcObjects.Get("My Object Name");
		AssertPointer(pNamed1.Object(), pTemp.Object());

		sLogConfig = gcLogger.SetSilent();
		pNamed2 = gcObjects.AllocateNamedUninitialisedByClassNameAndAddIntoMemory("CTestNamedObject", "My Object Name");
		gcLogger.SetConfig(&sLogConfig);

		AssertNull(pNamed2.Object());
		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertLong(NULL_O_INDEX, pNamed2.GetIndex());

		AssertLong(1LL, pNamed1.GetIndex());
		AssertString("My Object Name", pNamed1.GetName());
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorNamedOverwrite(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CTestNamedObject>	pNamed1;
		Ptr<CTestNamedObject>	pNamed2;
		Ptr<CTestNamedObject>	pTemp;

		TestObjectAllocatorAddConstructors();

		AssertLong(0, gcObjects.NumMemoryIndexes());

		pNamed1 = gcObjects.AllocateNamedUninitialisedByClassNameAndAddIntoMemory("CTestNamedObject", "My Object Name");
		pNamed1->Init(1);
		AssertLong(1LL, pNamed1.GetIndex());
		AssertString("My Object Name", pNamed1.GetName());

		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed2 = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObject", "My Object Name");
		pNamed2->Init(2);
		AssertNotNull(pNamed2.Object());
		AssertLong(2LL, pNamed2.GetIndex());
		AssertString("My Object Name", pNamed2.GetName());
		AssertLong(1, gcObjects.NumMemoryIndexes());

		pNamed1 = gcObjects.Get(2LL);
		AssertNotNull(pNamed1.Object());
		AssertInt((int32)2, pNamed1->miNum);
		AssertInt((int32)2, pNamed2->miNum);
		AssertPointer(pNamed2.Object(), pNamed1.Object());

		AssertLong(1, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pTemp = gcObjects.Get("My Object Name");
		AssertPointer(pNamed2.Object(), pTemp.Object());
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


size guiDestroyed;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorCountDestructions(CBaseObject* pvObject)
{
	guiDestroyed++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorOverwriteFromRootCausesChildrenToBeDestroyed(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestNamedObject>		pNamed1;
		Ptr<CTestNamedObject>		pNamed2;
		Ptr<CTestNamedObject>		pNamed3;
		Ptr<CTestNamedObject>		pNamed4;
		Ptr<CTestNamedObject>		pNamed5;
		CPointer					pObject1;
		CPointer					pObject2;
		Ptr<CTestNamedObjectSmall>	pNamedSmall;
		CChars						sz;

		guiDestroyed = 0;
		gcObjects.SetDestructionCallback(&TestObjectAllocatorCountDestructions);

		TestObjectAllocatorAddConstructors();


		//          "4"(6)  "3"(5)  "5"(7)
		//            |     /   \     |
		//            |    /     \    |
		//            |   /       \   |
		//            |  /         \  |
		//            | /           \ |
		//           "1"(3)       "2"(4)
		//              \           /
		//               \         /
		//                \       /
		//                 \     /
		//                  \   /
		//                   ...
		//                  Root(0)

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed1 = ONMalloc<CTestNamedObject>("1", 1);
		pRoot->Add(pNamed1);

		pNamed2 = ONMalloc<CTestNamedObject>("2", 2);
		pRoot->Add(pNamed2);

		pNamed3 = ONMalloc<CTestNamedObject>("3", 3);
		pNamed1->mpNamedTest1 = pNamed3;
		pNamed2->mpNamedTest1 = pNamed3;

		Pass();
		pNamed4 = ONMalloc<CTestNamedObject>("4", 4);
		pNamed1->mpNamedTest2 = pNamed4;

		pNamed5 = ONMalloc<CTestNamedObject>("5", 5);
		pNamed2->mpNamedTest2 = pNamed5;

		AssertLong(1, pRoot.GetIndex());
		AssertLong(3, pNamed1.GetIndex());
		AssertLong(4, pNamed2.GetIndex());
		AssertLong(5, pNamed3.GetIndex());
		AssertLong(6, pNamed4.GetIndex());
		AssertLong(7, pNamed5.GetIndex());

		pNamed3 = NULL;
		pNamed4 = NULL;
		pNamed5 = NULL;

		AssertLong(7, gcObjects.NumMemoryIndexes());
		AssertLong(6, gcObjects.NumMemoryNames());

		AssertString("CTestNamedObject", pNamed1.ClassName());
		AssertString("CTestNamedObject", pNamed2.ClassName());
		AssertLong(3, pNamed1.GetIndex());
		AssertLong(4, pNamed2.GetIndex());

		sz.Init();
		gcObjects.PrintMemoryNames(&sz);
		AssertString(""\
			"1\n"\
			"2\n"\
			"3\n"\
			"4\n"\
			"5\n"\
			"GraphRoot\n", sz.Text());
		sz.Kill();

		AssertSize(0, guiDestroyed);

		//          "4"(6)  "3"(5)  "5"(7)
		//            |     / 
		//            |    /  
		//            |   /   
		//            |  /    
		//            | /     
		//           "1"(3)       "2"(4)
		//              \           /
		//               \         /
		//                \       /
		//                 \     /
		//                  \   /
		//                   ...
		//                  Root(0)

		guiDestroyed = 0;

		pNamedSmall = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObjectSmall", "2");
		AssertSize(2, guiDestroyed);
		pNamedSmall->Init("ABC");

		AssertString("CTestNamedObject", pNamed1.ClassName());
		AssertString("CTestNamedObjectSmall", pNamed2.ClassName());
		AssertLong(3LL, pNamed1.GetIndex());
		AssertLong(8LL, pNamed2.GetIndex());
		AssertString("CTestNamedObjectSmall", pNamedSmall.ClassName());
		AssertLong(8LL, pNamedSmall.GetIndex());

		pObject1 = pRoot->Get("1");
		pObject2 = pRoot->Get("2");

		AssertPointer(pObject1.Dereference(), pNamed1.Dereference());
		AssertPointer(pObject2.Dereference(), pNamedSmall.Dereference());

		AssertLong(6, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcObjects.NumMemoryNames());

		sz.Init();
		gcObjects.PrintMemoryNames(&sz);
		AssertString(""\
			"1\n"\
			"2\n"\
			"3\n"\
			"4\n"\
			"GraphRoot\n", sz.Text());
		sz.Kill();

		pNamed3 = gcObjects.Get("3");
		pNamed4 = gcObjects.Get("4");
		pNamed5 = gcObjects.Get("5");

		AssertNotNull(&pNamed3);
		AssertNotNull(&pNamed4);
		AssertNull(&pNamed5);

		guiDestroyed = 0;
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorNullCausesChildrenToBeDestroyed(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestNamedObject>		pNamed1;
		Ptr<CTestNamedObject>		pNamed2;
		Ptr<CTestNamedObject>		pNamed3;
		Ptr<CTestNamedObject>		pNamed4;
		Ptr<CTestNamedObject>		pNamed5;
		CPointer					pObject1;
		CPointer					pObject2;
		CChars						sz;

		guiDestroyed = 0;
		gcObjects.SetDestructionCallback(&TestObjectAllocatorCountDestructions);

		TestObjectAllocatorAddConstructors();


		//          "4"(6)  "3"(5)  "5"(7)
		//            |     /   \     |
		//            |    /     \    |
		//            |   /       \   |
		//            |  /         \  |
		//            | /           \ |
		//           "1"(3)       "2"(4)
		//              \           /
		//               \         /
		//                \       /
		//                 \     /
		//                  \   /
		//                   ...
		//                  Root(0)

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed1 = ONMalloc<CTestNamedObject>("1", 1);
		pRoot->Add(pNamed1);

		pNamed2 = ONMalloc<CTestNamedObject>("2", 2);
		pRoot->Add(pNamed2);

		pNamed3 = ONMalloc<CTestNamedObject>("3", 3);
		pNamed1->mpNamedTest1 = pNamed3;
		pNamed2->mpNamedTest1 = pNamed3;

		Pass();
		pNamed4 = ONMalloc<CTestNamedObject>("4", 4);
		pNamed1->mpNamedTest2 = pNamed4;

		pNamed5 = ONMalloc<CTestNamedObject>("5", 5);
		pNamed2->mpNamedTest2 = pNamed5;

		AssertLong(1, pRoot.GetIndex());
		AssertLong(3, pNamed1.GetIndex());
		AssertLong(4, pNamed2.GetIndex());
		AssertLong(5, pNamed3.GetIndex());
		AssertLong(6, pNamed4.GetIndex());
		AssertLong(7, pNamed5.GetIndex());

		pNamed3 = NULL;
		pNamed4 = NULL;
		pNamed5 = NULL;

		AssertLong(7, gcObjects.NumMemoryIndexes());
		AssertLong(6, gcObjects.NumMemoryNames());

		AssertString("CTestNamedObject", pNamed1.ClassName());
		AssertString("CTestNamedObject", pNamed2.ClassName());
		AssertLong(3, pNamed1.GetIndex());
		AssertLong(4, pNamed2.GetIndex());

		sz.Init();
		gcObjects.PrintMemoryNames(&sz);
		AssertString(""\
			"1\n"\
			"2\n"\
			"3\n"\
			"4\n"\
			"5\n"\
			"GraphRoot\n", sz.Text());
		sz.Kill();

		AssertSize(0, guiDestroyed);

		//          "4"(6)  "3"(5)  "5"(7)
		//            |     / 
		//            |    /  
		//            |   /   
		//            |  /    
		//            | /     
		//           "1"(3)       "2"(4)
		//              \
		//               \
		//                \
		//                 \
		//                  \
		//                   ...
		//                  Root(0)

		guiDestroyed = 0;

		pRoot->Remove(pNamed2);
		pNamed2 = NULL;
		AssertSize(2, guiDestroyed);

		AssertLong(3, pNamed1.GetIndex());
		AssertLong(NULL_O_INDEX, pNamed2.GetIndex());

		pObject1 = pRoot->Get("1");
		pObject2 = pRoot->Get("2");

		AssertPointer(pObject1.Dereference(), pNamed1.Dereference());
		AssertNull(pObject2.Dereference());

		AssertLong(5, gcObjects.NumMemoryIndexes());
		AssertLong(4, gcObjects.NumMemoryNames());

		sz.Init();
		gcObjects.PrintMemoryNames(&sz);
		AssertString(""\
			"1\n"\
			"3\n"\
			"4\n"\
			"GraphRoot\n", sz.Text());
		sz.Kill();

		pNamed3 = gcObjects.Get("3");
		pNamed4 = gcObjects.Get("4");
		pNamed5 = gcObjects.Get("5");

		AssertNotNull(&pNamed3);
		AssertNotNull(&pNamed4);
		AssertNull(&pNamed5);

		guiDestroyed = 0;
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorAssignmentToNullObject(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CRoot>				pRoot;
		Ptr<CTestNamedObject>	pNamed1;
		Ptr<CTestNamedObject>	pNamed2;
		char					szLog[4096];
		char* szError;
		CLogToMemory			cLogToMemory;


		TestObjectAllocatorAddConstructors();

		pRoot = ORoot();

		pNamed1 = ONMalloc<CTestNamedObject>("1", 1);
		pRoot->Add(pNamed1);


		cLogToMemory.Start(true);

		//gcLogger.SetBreakOnError(false);
		pNamed2->mpNamedTest1 = pNamed1;

		cLogToMemory.Stop(szLog, 4096);

		szError = strstr(szLog, "Attempted to dereference NULL Pointer.");
		AssertNotNull(szError);


		AssertLong(3, gcObjects.NumMemoryIndexes());
		AssertLong(2, gcObjects.NumMemoryNames());
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorOverwrittensParentMaintainsPointerToOverwritten(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CTestNamedObject>		pNamed1;
		Ptr<CTestNamedObject>		pNamed2;
		Ptr<CTestNamedObject>		pNamed3;
		Ptr<CTestNamedObject>		pNamed4;
		Ptr<CTestNamedObject>		pNamed5;
		Ptr<CTestNamedObject>		pNamed6;
		CPointer					pObject1;
		CPointer					pObject2;
		Ptr<CTestNamedObject>		pNamed3New;
		Ptr<CTestNamedObject>		pNamed4New;
		void* pv3;
		void* pv4;

		TestObjectAllocatorAddConstructors();

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed1 = ONMalloc<CTestNamedObject>("1", 1);
		pRoot->Add(pNamed1);

		pNamed2 = ONMalloc<CTestNamedObject>("2", 2);
		pRoot->Add(pNamed2);

		pNamed3 = ONMalloc<CTestNamedObject>("3", 3);
		pNamed4 = ONMalloc<CTestNamedObject>("4", 4);
		pNamed5 = ONMalloc<CTestNamedObject>("5", 5);
		pNamed6 = ONMalloc<CTestNamedObject>("6", 6);

		pNamed1->mpNamedTest1 = pNamed3;
		pNamed3->mpNamedTest1 = pNamed5;
		pNamed3->mpNamedTest2 = pNamed6;

		pNamed2->mpNamedTest1 = pNamed4;
		pNamed4->mpNamedTest1 = pNamed6;
		pNamed4->mpNamedTest2 = pNamed5;

		AssertInt((int32)3, pNamed1->mpNamedTest1->miNum);
		AssertInt((int32)4, pNamed2->mpNamedTest1->miNum);

		AssertLong(8, gcObjects.NumMemoryIndexes());
		AssertLong(7, gcObjects.NumMemoryNames());

		AssertInt(3, pNamed3->GetDistToRoot());
		AssertInt(3, pNamed4->GetDistToRoot());
		AssertLong(5LL, pNamed3->GetIndex());
		AssertLong(6LL, pNamed4->GetIndex());

		pv3 = &pNamed3;
		pv4 = &pNamed4;

		pNamed3New = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObject", "3");
		pNamed3New->Init(33);
		pNamed4New = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObject", "4");
		pNamed4New->Init(44);

		AssertLong(8, gcObjects.NumMemoryIndexes());
		AssertLong(7, gcObjects.NumMemoryNames());

		AssertLong(9LL, pNamed3New->GetIndex());
		AssertLong(10ll, pNamed4New->GetIndex());

		AssertInt((int32)33, pNamed1->mpNamedTest1->miNum);
		AssertInt((int32)44, pNamed2->mpNamedTest1->miNum);

		AssertTrue(&pNamed3 == &pNamed3New);
		AssertTrue(&pNamed4 == &pNamed4New);
		AssertFalse(pv3 == &pNamed3New);
		AssertFalse(pv4 == &pNamed4New);

		AssertInt(3, pNamed3New->GetDistToRoot());
		AssertInt(3, pNamed4New->GetDistToRoot());
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorOverwrittensParentMaintainsPointerToOverwrittenWithEmbedded(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	{
		Ptr<CRoot>							pRoot;
		Ptr<CTestNamedObjectWithEmbedded>	pNamed1;
		Ptr<CTestNamedObjectWithEmbedded>	pNamed2;
		Ptr<CTestNamedObjectSmall>			pNamed1a;
		Ptr<CTestNamedObject>				pNamed1b;
		Ptr<CTestNamedObjectSmall>			pNamed2a;
		Ptr<CTestNamedObject>				pNamed2b;

		TestObjectAllocatorAddConstructors();

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed1a = ONMalloc<CTestNamedObjectSmall>("TestNamedObjectSmall 1A", (char*)"ABC");
		pNamed1b = ONMalloc<CTestNamedObject>("TestNamedObject 1B", 4);
		pNamed2a = ONMalloc<CTestNamedObjectSmall>("TestNamedObjectSmall 2A", (char*)"XYZ");
		pNamed2b = ONMalloc<CTestNamedObject>("TestNamedObject 2B", 4);

		pNamed1 = ONMalloc<CTestNamedObjectWithEmbedded>("Agrarian", 12, 13, 14, 15, pNamed1b, pNamed1a);
		pRoot->Add(pNamed1);

		pNamed2 = ONMalloc<CTestNamedObjectWithEmbedded>("Hydroponics", 2012, 2013, 2014, 2015, pNamed2b, pNamed2a);
		pRoot->Add(pNamed2);

		pNamed2a = NULL;
		pNamed2b = NULL;

		AssertLong(8, gcObjects.NumMemoryIndexes());
		AssertLong(7, gcObjects.NumMemoryNames());

		AssertInt(2, pNamed1->GetDistToRoot());
		AssertInt(3, pNamed1->mpObject->GetDistToRoot());
		AssertInt(3, pNamed1->mpSmall->GetDistToRoot());
		AssertInt(3, pNamed2->mpObject->GetDistToRoot());
		AssertInt(3, pNamed2->mpSmall->GetDistToRoot());
		AssertInt(2, pNamed1->mNamedTest1.GetDistToRoot());
		AssertInt(2, pNamed1->mNamedTest2.GetDistToRoot());
		AssertInt(2, pNamed2->mNamedTest1.GetDistToRoot());
		AssertInt(2, pNamed2->mNamedTest2.GetDistToRoot());


		pNamed1 = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObjectWithEmbedded", "Agrarian");
		pNamed1->Init(1012, 1013, 1014, 1015, Null(), Null());
		pNamed2 = gcObjects.GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName("CTestNamedObjectWithEmbedded", "Hydroponics");
		pNamed2->Init(2012, 2013, 2014, 2015, Null(), Null());

		AssertLong(6, gcObjects.NumMemoryIndexes());
		AssertLong(5, gcObjects.NumMemoryNames());

		AssertInt(2, pNamed1->GetDistToRoot());
		AssertNull(&pNamed1->mpObject);
		AssertNull(&pNamed1->mpSmall);
		AssertNull(&pNamed2->mpObject);
		AssertNull(&pNamed2->mpSmall);
		AssertInt(2, pNamed1->mNamedTest1.GetDistToRoot());
		AssertInt(2, pNamed1->mNamedTest2.GetDistToRoot());
		AssertInt(2, pNamed2->mNamedTest1.GetDistToRoot());
		AssertInt(2, pNamed2->mNamedTest2.GetDistToRoot());

		AssertInt(-1, pNamed1a->GetDistToRoot());
		AssertInt(-1, pNamed1b->GetDistToRoot());

		pNamed1 = NULL;
		pNamed2 = NULL;
		pNamed1a = NULL;
		pNamed2a = NULL;
		pNamed1b = NULL;
		pNamed2b = NULL;

		AssertLong(4, gcObjects.NumMemoryIndexes());
		AssertLong(3, gcObjects.NumMemoryNames());

		pNamed1 = gcObjects.Get("Agrarian");
		pNamed2 = gcObjects.Get("Hydroponics");
		AssertNotNull(&pNamed1);
		AssertNotNull(&pNamed2);
	}
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocatorOverwrittensFlushedObjects(void)
{
	CFileUtil							cFileUtil;
	char								szDirectory[] = "Output" _FS_ "ObjectAllocator" _FS_ "Database1";

	cFileUtil.RemoveDir(szDirectory);
	MemoryInit();
	TypesInit();
	DataIOInit();
	{
		Ptr<CRoot>							pRoot;
		Ptr<CTestTriPointerObject>			pObject1;
		Ptr<CTestNamedObject>				pNamed2;
		Ptr<CTestObject>					pObject3;
		Ptr<CTestNamedObjectSmall>			pNamed4;
		Ptr<CTestNamedObjectWithEmbedded>	pNamed5;
		Ptr<CTestObject>					pObject6;
		Ptr<CTestNamedObject>				pNamed7;
		Ptr<CTestNamedObject>				pNamed8;
		CCodabase* pcDatabase;
		CSequence* pcSequence;

		pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
		pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
		pcDatabase->Open();
		ObjectsInit(pcDatabase, pcSequence);

		TestObjectAllocatorAddConstructors();

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pObject1 = OMalloc<CTestTriPointerObject>();
		pNamed2 = ONMalloc<CTestNamedObject>("Virgil Mallin", 12);
		pObject3 = OMalloc<CTestObject>();	pObject3->mi = 34;
		pNamed4 = ONMalloc<CTestNamedObjectSmall>("Henry Lindsey", "Sol");
		pNamed5 = ONMalloc<CTestNamedObjectWithEmbedded>("Wilfrid Stanley", 44, 55, 1, 2, Null(), Null());
		pObject6 = OMalloc<CTestObject>();	pObject3->mi = 56;
		pNamed7 = ONMalloc<CTestNamedObject>("Matt Hudson", 78);
		pNamed8 = ONMalloc<CTestNamedObject>("Kiva Charity", 90);

		AssertInt(-1, pObject1.GetDistToRoot());
		AssertInt(-1, pNamed2.GetDistToRoot());
		AssertInt(-1, pObject3.GetDistToRoot());
		AssertInt(-1, pNamed4.GetDistToRoot());
		AssertInt(-1, pNamed5.GetDistToRoot());
		AssertInt(-1, pObject6.GetDistToRoot());
		AssertInt(-1, pNamed7.GetDistToRoot());

		pRoot->Add(pObject1);
		pObject1->mpObject1 = pNamed5;
		pObject1->mpObject2 = pObject3;
		pObject1->mpObject3 = pNamed4;

		pNamed5->mNamedTest1.mpNamedTest1 = pNamed2;
		pNamed5->mNamedTest1.mpNamedTest2 = pNamed7;
		pNamed5->mpObject = pObject6;

		pObject6->mpObject = pNamed4;

		//	                                      
		//	                                     pNamed4(3)
		//	                                      /   |     
		//          pNamed2(4)   pNamed7(4)      /    |
		//	           |            |           /     |   
		//	           |            |   pObject6(4)   |
		//             |            |     /           |
		//	           |            |    /            |       
		//             |            |   /             |        
		//	           |            |  /              |      
		//          mNamedTest1  mNamedTest2          |
		//                    .  .   /               /
		//                  pNamed5(3)  pObject3(3) /
		// 	                       \       |       /  
		// 	                        \      |      /  
		//                           \     |     /  
		// 	                          \    |    /  
		//                             \   |   /   
		// 	                            \  |  /    
		//                             pObject1(2)
		//                                 |
		//                                 |
		//                                ...
		//                              Root(0)                   pNamed8(-1)  


		AssertInt(0, pRoot.GetDistToRoot());
		AssertInt(2, pObject1.GetDistToRoot());
		AssertInt(4, pNamed2.GetDistToRoot());
		AssertInt(3, pObject3.GetDistToRoot());
		AssertInt(3, pNamed4.GetDistToRoot());
		AssertInt(3, pNamed5.GetDistToRoot());
		AssertInt(4, pObject6.GetDistToRoot());
		AssertInt(4, pNamed7.GetDistToRoot());
		AssertInt(-1, pNamed8.GetDistToRoot());

		AssertLong(10, gcObjects.NumMemoryIndexes());
		AssertLong(6, gcObjects.NumMemoryNames());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
		ObjectsKill(false);

		AssertNull(&pRoot);
		AssertNull(&pObject1);
		AssertNull(&pNamed2);
		AssertNull(&pObject3);
		AssertNull(&pNamed4);
		AssertNull(&pNamed5);
		AssertNull(&pObject6);
		AssertNull(&pNamed7);
		AssertNull(&pNamed8);

		pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
		pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
		pcDatabase->Open();
		ObjectsInit(pcDatabase, pcSequence);

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());
		AssertLong(9, gcObjects.NumDatabaseIndexes());
		AssertLong(5, gcObjects.NumDatabaseNames());
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());

		pNamed2 = gcObjects.Get("Virgil Mallin");
		AssertSize(0, pNamed2.NumHeapFroms());
		AssertInt(4, pNamed2.GetDistToRoot());
		pNamed4 = gcObjects.Get("Henry Lindsey");
		AssertSize(0, pNamed4.NumHeapFroms());
		AssertInt(3, pNamed4.GetDistToRoot());
		pNamed5 = gcObjects.Get("Wilfrid Stanley");
		AssertSize(0, pNamed5.NumHeapFroms());
		AssertInt(3, pNamed5.GetDistToRoot());
		pNamed7 = gcObjects.Get("Matt Hudson");
		AssertSize(1, pNamed7.NumHeapFroms());
		AssertInt(4, pNamed7.GetDistToRoot());
		pNamed8 = gcObjects.Get("Kiva Charity");
		AssertSize(0, pNamed8.NumHeapFroms());
		AssertInt(-1, pNamed8.GetDistToRoot());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
		ObjectsKill(false);


		pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
		pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
		pcDatabase->Open();
		ObjectsInit(pcDatabase, pcSequence);

		pRoot = ORoot();
		AssertLong(2, gcObjects.NumMemoryIndexes());
		AssertLong(1, gcObjects.NumMemoryNames());
		AssertLong(9, gcObjects.NumDatabaseIndexes());
		AssertLong(5, gcObjects.NumDatabaseNames());

		ObjectsFlush();
		pcDatabase->Close();
		SafeKill(pcDatabase);
		SafeKill(pcSequence);
	}
	ObjectsKill();


	DataIOKill();
	TypesKill();
	MemoryKill();
	cFileUtil.RemoveDir(szDirectory);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectAllocator(void)
{
	BeginTests();

	TestObjectAllocatorSimpleAdd();
	TestObjectAllocatorNamedAdd();
	TestObjectAllocatorNamedOverwrite();
	TestObjectAllocatorNullCausesChildrenToBeDestroyed();
	TestObjectAllocatorOverwriteFromRootCausesChildrenToBeDestroyed();
	TestObjectAllocatorAssignmentToNullObject();
	TestObjectAllocatorOverwrittensParentMaintainsPointerToOverwritten();
	TestObjectAllocatorOverwrittensParentMaintainsPointerToOverwrittenWithEmbedded();
	TestObjectAllocatorOverwrittensFlushedObjects();

	// Test dependent named objects overwrite existing named objects.  i.e:  Load "Name 1" -> x -> "Name 2" overwrites "Name 2" that exists in the database.

	TestStatistics();
}

