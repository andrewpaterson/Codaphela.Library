#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "BaseLib/NamedIndexedHeader.h"
#include "StandardLib/Objects.h"
#include "StandardLib/FreeObjects.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"
#include "UnknownObjectHolderTestClasses.h"


#ifdef BUILD_64
#define GRAPH_ROOT_SIZE					"368"
#define GRAPH_SET_OBJECT_SIZE			"352"
#define GRAPH_OBJECT_SIZE				"344"
#define GRAPH_TEST_OBJECT_SIZE			"392"
#define GRAPH_OBJECT_WITH_UNKNOWN_SIZE	"360"
#elif defined(BASELIB_SIZE16)
#define GRAPH_ROOT_SIZE					"168"
#define GRAPH_SET_OBJECT_SIZE			"168"
#define GRAPH_OBJECT_SIZE				"160"
#define GRAPH_TEST_OBJECT_SIZE			"184"
#define GRAPH_OBJECT_WITH_UNKNOWN_SIZE	"168"
#elif defined(__W65C816__)
#define GRAPH_ROOT_SIZE					"176"
#define GRAPH_SET_OBJECT_SIZE			"168"
#define GRAPH_OBJECT_SIZE				"160"
#define GRAPH_TEST_OBJECT_SIZE			"184"
#define GRAPH_OBJECT_WITH_UNKNOWN_SIZE	"168"
#else
#define GRAPH_ROOT_SIZE					"200"
#define GRAPH_SET_OBJECT_SIZE			"192"
#define GRAPH_OBJECT_SIZE				"184"
#define GRAPH_TEST_OBJECT_SIZE			"208"
#define GRAPH_OBJECT_WITH_UNKNOWN_SIZE	"192"
#endif // BUILD_64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFreeUnknownOnly(void)
{
	Ptr<CFreeObjects>		pFree;
	CUnknownObjectHolder	cUnknown;
	CChars					sz;

	ObjectsInit();
	{
		ORoot();
		AssertSize(2, gcObjects.NumMemoryIndexes());
		cUnknown.Init(NULL);
		AssertSize(5, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,1\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObject(" GRAPH_OBJECT_SIZE ") Index:4 Name:GraphFreeObjects Froms:1,0\n\
          [ 3]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:5 Froms:1,0\n\
             [ 4]:CTestObject(" GRAPH_TEST_OBJECT_SIZE ") Index:3 Froms:1,0\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();

		cUnknown.Kill();
		AssertSize(4, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,1\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObject(" GRAPH_OBJECT_SIZE ") Index:4 Name:GraphFreeObjects Froms:1,0\n\
          [ 3]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:5 Froms:1,0\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();	
	}
	ObjectsFlush();
	ObjectsKill(false);
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFreeRemoveObjectFromRootBeforeNull(void)
{
	STestObjectFreedNotifier	sFreedNotifier;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CTestObject>	pObject;

		pRoot = ORoot();
		AssertSize(2, gcObjects.NumMemoryIndexes());
		pObject = OMalloc<CTestObject>(&sFreedNotifier);
		pRoot->Add(pObject);

		AssertSize(3, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();

		pRoot->Remove(pObject);
		gcObjects.ValidateObjectsConsistency();

		pObject = NULL;

		AssertSize(2, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill(false);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFreeUnknownInObjectAssignNull(void)
{
	CChars						sz;
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CObjectWithUnknown>		pObjectWithUnknown;

		pRoot = ORoot();
		AssertSize(2, gcObjects.NumMemoryIndexes());
		pObjectWithUnknown = OMalloc<CObjectWithUnknown>(&sFreedNotifier1);
		pRoot->Add(pObjectWithUnknown);

		AssertSize(3, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,2\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObjectWithUnknown(" GRAPH_OBJECT_WITH_UNKNOWN_SIZE ") Index:3 Froms:1,1\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();

		pObjectWithUnknown->AllocateUnknown(&sFreedNotifier2);

		AssertSize(6, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,2\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObjectWithUnknown(" GRAPH_OBJECT_WITH_UNKNOWN_SIZE ") Index:3 Froms:1,1\n\
       [ 2]:CObject(" GRAPH_OBJECT_SIZE ") Index:5 Name:GraphFreeObjects Froms:1,0\n\
          [ 3]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:6 Froms:1,0\n\
             [ 4]:CTestObject(" GRAPH_TEST_OBJECT_SIZE ") Index:4 Froms:1,0\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();
		gcObjects.ValidateObjectsConsistency();

		pRoot->Remove(pObjectWithUnknown);

		AssertSize(6, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,2\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObject(" GRAPH_OBJECT_SIZE ") Index:5 Name:GraphFreeObjects Froms:1,0\n\
          [ 3]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:6 Froms:1,0\n\
             [ 4]:CTestObject(" GRAPH_TEST_OBJECT_SIZE ") Index:4 Froms:1,0\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();
		gcObjects.ValidateObjectsConsistency();

		pObjectWithUnknown = NULL;
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFreeUnknownInObjectPointerDestructor(void)
{
	CChars						sz;
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CObjectWithUnknown>		pObjectWithUnknown;

		pRoot = ORoot();
		AssertSize(2, gcObjects.NumMemoryIndexes());
		pObjectWithUnknown = OMalloc<CObjectWithUnknown>(&sFreedNotifier1);
		pRoot->Add(pObjectWithUnknown);

		AssertSize(3, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,2\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObjectWithUnknown(" GRAPH_OBJECT_WITH_UNKNOWN_SIZE ") Index:3 Froms:1,1\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();

		pObjectWithUnknown->AllocateUnknown(&sFreedNotifier2);

		AssertSize(6, gcObjects.NumMemoryIndexes());
		sz.Init();
		gcObjects.PrintGraph(&sz, false);
		AssertString("\
-------------------------- Graph --------------------------- \n\
 [ 0]:CRoot(" GRAPH_ROOT_SIZE ") Index:1 Name:GraphRoot Froms:0,2\n\
    [ 1]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:2 Froms:1,0\n\
       [ 2]:CObjectWithUnknown(" GRAPH_OBJECT_WITH_UNKNOWN_SIZE ") Index:3 Froms:1,1\n\
       [ 2]:CObject(" GRAPH_OBJECT_SIZE ") Index:5 Name:GraphFreeObjects Froms:1,0\n\
          [ 3]:CSetObject(" GRAPH_SET_OBJECT_SIZE ") Index:6 Froms:1,0\n\
             [ 4]:CTestObject(" GRAPH_TEST_OBJECT_SIZE ") Index:4 Froms:1,0\n\
------------------------------------------------------------ \n", sz.Text());
		sz.Kill();

		pRoot->Remove(pObjectWithUnknown);
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsFreeRootRemoveAll(void)
{
	STestObjectFreedNotifier	sNotifierRooted;
	STestObjectFreedNotifier	sNotifierFree;
	SLogConfig					sLogConfig;

	ObjectsInit();
	{
		Ptr<CRoot>			pRoot;
		Ptr<CTestObject>	pRooted;
		Ptr<CTestObject>	pFree;
		CTestObject*		pcFree;
		bool				bResult;

		pRoot = ORoot();
		pRooted = OMalloc<CTestObject>(&sNotifierRooted);
		pRoot->Add(pRooted);
		pRooted = NULL;

		pFree = OMalloc<CTestObject>(&sNotifierFree);
		pcFree = gcObjects.AddFreeObject<CTestObject>(pFree);
		pFree = NULL;
		AssertSize(1, gcObjects.NumFreeObjects());
		AssertSize(2, pRoot->NumObjects());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		//Removing all from the Root would free the free Object out from under its owner.
		sLogConfig = gcLogger.SetSilent();
		bResult = pRoot->RemoveAll();
		gcLogger.SetConfig(&sLogConfig);
		AssertFalse(bResult);
		AssertFalse(sNotifierRooted.bFreed);
		AssertFalse(sNotifierFree.bFreed);
		AssertSize(1, gcObjects.NumFreeObjects());
		AssertSize(2, pRoot->NumObjects());
		AssertSize(6, gcObjects.NumMemoryIndexes());

		AssertTrue(gcObjects.RemoveFreeObject(pcFree));
		AssertTrue(sNotifierFree.bFreed);
		AssertSize(0, gcObjects.NumFreeObjects());

		AssertTrue(pRoot->RemoveAll());
		AssertTrue(sNotifierRooted.bFreed);
		AssertSize(0, pRoot->NumObjects());
		AssertSize(2, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeObjects(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsFreeRemoveObjectFromRootBeforeNull();
	TestObjectsFreeUnknownOnly();
	TestObjectsFreeUnknownInObjectAssignNull();
	TestObjectsFreeUnknownInObjectPointerDestructor();
	TestObjectsFreeRootRemoveAll();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

