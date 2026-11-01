#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"
#include "CollectionObjectTestClasses.h"


size	guiSetDirtyAfterFreed = 0;


class CTestDirtyAfterFreed : public CObject
{
CONSTRUCTABLE(CTestDirtyAfterFreed)
DESTRUCTABLE(CTestDirtyAfterFreed)
public:
	Ptr<>	mp;

	void Init(void)
	{
		PreInit();
		PostInit();
	}

	void Class(void) override
	{
		M_Pointer(mp);
	}

	void Free(void) override
	{
	}

	void SetDirty(bool bDirty) override
	{
		if (muiFlags & OBJECT_FLAGS_FREED)
		{
			//Only reached if a freed object is used.  Don't touch it any further.
			guiSetDirtyAfterFreed++;
			return;
		}
		CObject::SetDirty(bDirty);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeSelfCyclicPointer(void)
{
	ObjectsInit();
	{
		Ptr<CTestObject>						pObject;
		CArrayBaseObjectPtrE32		apcHeapFroms;
		CArrayStackPointerE32						apcStackFroms;
		CBaseObject*							pcObject1;
		CBaseObject*							pcObject2;
		CPointer*								pcPointer;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject = OMalloc<CTestObject>();
		pObject->mpTest = pObject;
		AssertSize(1, pObject.NumHeapFroms());

		pObject->mpObject = pObject;

		AssertSize(2, pObject.NumHeapFroms());
		apcHeapFroms.Init();
		pObject->GetHeapFroms(&apcHeapFroms);
		pcObject1 = *apcHeapFroms.Get(0);
		pcObject2 = *apcHeapFroms.Get(1);
		apcHeapFroms.Kill();

		AssertPointer(&pObject, pcObject1);
		AssertPointer(&pObject, pcObject2);

		AssertSize(1, pObject.NumStackFroms());
		apcStackFroms.Init();
		pObject->GetStackFroms(&apcStackFroms);
		pcPointer = apcStackFroms.Get(0)->u.pcPointer;
		apcStackFroms.Kill();
		AssertPointer(pObject.This(), pcPointer);

		AssertSize(1, gcObjects.NumMemoryIndexes());

		pObject = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeLoopCyclicPointer(void)
{
	ObjectsInit();
	{
		Ptr<CTestObject>						pObject;
		CArrayBaseObjectPtrE32		apcHeapFroms;
		CArrayStackPointerE32						apcStackFroms;
		CBaseObject*							pcObject1;
		CBaseObject*							pcObject2;
		CPointer*								pcPointer;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject = OMalloc<CTestObject>();
		pObject->AllocateTest();
		AssertSize(0, pObject.NumHeapFroms());
		AssertSize(2, gcObjects.NumMemoryIndexes());
		pObject->mpTest->mpTest = pObject;
		pcObject1 = &pObject->mpTest;

		AssertSize(1, pObject.NumHeapFroms());
		apcHeapFroms.Init();
		pObject->GetHeapFroms(&apcHeapFroms);
		pcObject2 = *apcHeapFroms.Get(0);
		apcHeapFroms.Kill();

		AssertPointer(pcObject1, pcObject2);

		AssertSize(1, pObject.NumStackFroms());
		apcStackFroms.Init();
		pObject->GetStackFroms(&apcStackFroms);
		pcPointer = apcStackFroms.Get(0)->u.pcPointer;
		apcStackFroms.Kill();
		AssertPointer(pObject.This(), pcPointer);

		pObject = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeLoopCollectionCyclicPointer(void)
{
	ObjectsInit();
	{
		Ptr<CTestArrayObject>					pObject;
		CArrayBaseObjectPtrE32		apcHeapFroms;
		CArrayStackPointerE32						apcStackFroms;
		CPointer*								pcPointer;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject = OMalloc<CTestArrayObject>(3);
		pObject->AllocateTest();
		AssertSize(0, pObject.NumHeapFroms());
		AssertSize(2, gcObjects.NumMemoryIndexes());
		pObject->Get(0)->mpTest = pObject;

		AssertSize(1, pObject.NumHeapFroms());
		apcHeapFroms.Init();
		pObject->GetHeapFroms(&apcHeapFroms);
		apcHeapFroms.Kill();

		AssertSize(1, pObject.NumStackFroms());
		apcStackFroms.Init();
		pObject->GetStackFroms(&apcStackFroms);
		pcPointer = apcStackFroms.Get(0)->u.pcPointer;
		apcStackFroms.Kill();
		AssertPointer(pObject.This(), pcPointer);

		pObject = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeStackWithCyclicHeapPointer(void)
{
	STestObjectFreedNotifier	sFreed1;
	STestObjectFreedNotifier	sFreed2;

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;
		CArrayStackPointerE32			apcStackFroms;
		CChars						sz;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject1 = OMalloc<CTestObject>(&sFreed1);
		pObject2 = OMalloc<CTestObject>(&sFreed2);

		pObject1->mpTest = pObject2;
		pObject2->mpTest = pObject1;
		
		AssertFalse(sFreed1.bFreed);
		AssertFalse(sFreed2.bFreed);
		AssertSize(1, pObject1.NumStackFroms());
		AssertSize(1, pObject2.NumStackFroms());
		AssertSize(2, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CTestObject:1 Ru Su (CC, D, CA, CI, CC)]\n\
1: Pointer [CTestObject:2 Ru Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

		pObject1 = NULL;

		AssertFalse(sFreed1.bFreed);
		AssertFalse(sFreed2.bFreed);
		AssertSize(0, pObject1.NumStackFroms());
		AssertSize(1, pObject2.NumStackFroms());
		AssertSize(2, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CTestObject:2 Ru Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

		pObject2 = NULL;

		AssertTrue(sFreed1.bFreed);
		AssertTrue(sFreed2.bFreed);
		AssertSize(0, pObject1.NumStackFroms());
		AssertSize(0, pObject2.NumStackFroms());
		AssertSize(0, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("", sz.Text());
		sz.Kill();
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeStackWithCyclicHeapPointerHeapFrom(void)
{
	STestObjectFreedNotifier	sFreed1;
	STestObjectFreedNotifier	sFreed2;
	STestObjectFreedNotifier	sFreedBase;

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;
		Ptr<CTestObject>			pObjectBase;
		CArrayStackPointerE32			apcStackFroms;
		CChars						sz;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject1 = OMalloc<CTestObject>(&sFreed1);
		pObject2 = OMalloc<CTestObject>(&sFreed2);
		pObjectBase = OMalloc<CTestObject>(&sFreedBase);

		pObject1->mpTest = pObject2;
		pObject2->mpTest = pObject1;
		pObjectBase->mpTest = pObject1;

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CTestObject:1 Ru Su (CC, D, CA, CI, CC)]\n\
1: Pointer [CTestObject:2 Ru Su (CC, D, CA, CI, CC)]\n\
2: Pointer [CTestObject:3 Ru Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

		pObject1 = NULL;
		pObject2 = NULL;

		AssertFalse(sFreed1.bFreed);
		AssertFalse(sFreed2.bFreed);
		AssertFalse(sFreedBase.bFreed);
		AssertSize(0, pObject1.NumStackFroms());
		AssertSize(0, pObject2.NumStackFroms());
		AssertSize(1, pObjectBase.NumStackFroms());
		AssertSize(3, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CTestObject:3 Ru Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeStackWithCyclicHeapPointerRootFrom(void)
{
	STestObjectFreedNotifier	sFreed1;
	STestObjectFreedNotifier	sFreed2;

	ObjectsInit();
	{
		Ptr<CTestObject>			pObject1;
		Ptr<CTestObject>			pObject2;
		Ptr<CRoot>					pRoot;
		CArrayStackPointerE32			apcStackFroms;
		CChars						sz;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		pObject1 = OMalloc<CTestObject>(&sFreed1);
		pObject2 = OMalloc<CTestObject>(&sFreed2);
		
		pRoot = ORoot();

		pObject1->mpTest = pObject2;
		pObject2->mpTest = pObject1;
		pRoot->Add(pObject1);

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		if (sz.Equals("\
0: Pointer [CTestObject:1 R2 Su (CC, D, CA, CI, CC)]\n\
1: Pointer [CTestObject:2 R3 Su (CC, D, CA, CI, CC)]\n\
2: Pointer [CRoot:3:\"GraphRoot\" R0 Su (CC, D, CA, CI, CC)]\n") 
			||
			sz.Equals("\
0: Pointer [CRoot:3:\"GraphRoot\" R0 Su (CC, D, CA, CI, CC)]\n\
1: Pointer [CTestObject:1 R2 Su (CC, D, CA, CI, CC)]\n\
2: Pointer [CTestObject:2 R3 Su (CC, D, CA, CI, CC)]\n"))
		{
			Pass();
		}
		else
		{
			Fail();
		}
		sz.Kill();

		pObject1 = NULL;
		pObject2 = NULL;

		AssertFalse(sFreed1.bFreed);
		AssertFalse(sFreed2.bFreed);
		AssertSize(0, pObject1.NumStackFroms());
		AssertSize(0, pObject2.NumStackFroms());
		AssertSize(4, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CRoot:3:\"GraphRoot\" R0 Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

		pRoot = NULL;

		AssertFalse(sFreed1.bFreed);
		AssertFalse(sFreed2.bFreed);
		AssertSize(4, gcObjects.NumMemoryIndexes());

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("", sz.Text());
		sz.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeEmbeddingFreedByPointTo(void)
{
	ObjectsInit();
	{
		Ptr<CTestDirtyAfterFreed>	pA;
		Ptr<CTestDirtyAfterFreed>	pOld;
		CTestDirtyAfterFreed*		pcA;

		guiSetDirtyAfterFreed = 0;

		pA = OMalloc<CTestDirtyAfterFreed>();
		pOld = OMalloc<CTestDirtyAfterFreed>();
		pA->mp = pOld;
		pOld->mp = pA;
		pOld = NULL;
		AssertSize(2, gcObjects.NumMemoryIndexes());

		//
		//  A <--> Old
		//  |
		//  pA (stack)
		//

		//Return leaves A alive with no stack froms.  Nothing holds A or Old but each other.
		pcA = (CTestDirtyAfterFreed*)pA.Return();
		AssertSize(0, pcA->NumStackFroms());
		AssertSize(2, gcObjects.NumMemoryIndexes());

		//Removing A's pointer to Old frees Old, and A with it.  PointTo must not use A after that.
		pcA->mp = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertSize(0, guiSetDirtyAfterFreed);
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFree(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectFreeSelfCyclicPointer();
	TestObjectFreeLoopCyclicPointer();
	TestObjectFreeLoopCollectionCyclicPointer();
	TestObjectFreeStackWithCyclicHeapPointer();
	TestObjectFreeStackWithCyclicHeapPointerHeapFrom();
	TestObjectFreeStackWithCyclicHeapPointerRootFrom();
	TestObjectFreeEmbeddingFreedByPointTo();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

