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
void TestHollowObjectAddConstructors(void)
{
	gcObjects.AddConstructor<CTestObject>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHollowObjectAllocation(void)
{
	CFileUtil				cFileUtil;
	Ptr<CTestObject>		pObject1;
	Ptr<CTestObject>		pObject2;
	Ptr<CTestObject>		pObject3;
	Ptr<CRoot>				pRoot;
	CCodabase*				pcDatabase;
	CSequence*				pcSequence;
	CArrayBaseObjectPtrE32	apcFroms;
	char					szDirectory[] = "Output" _FS_ "HollowObject1";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	TestHollowObjectAddConstructors();

	pRoot = ORoot();
	pObject1 = ONMalloc<CTestObject>("Pavel");
	pObject2 = ONMalloc<CTestObject>("Meninski");
	pObject3 = ONMalloc<CTestObject>("Ned");
	pRoot->Add(pObject1);
	pObject1->mpTest = pObject2;
	pObject2->mpTest = pObject3;
	
	AssertTrue(pObject1->IsDirty());
	AssertTrue(pObject2->IsDirty());
	AssertTrue(pObject3->IsDirty());
	ObjectsFlush();

	AssertFalse(pObject1->IsDirty());
	AssertFalse(pObject2->IsDirty());
	AssertFalse(pObject3->IsDirty());
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	pRoot = ORoot();
	pRoot->TouchAll();

	AssertNull(&pObject1);
	pObject1 = gcObjects.Get("Pavel");
	AssertNotNull(&pObject1);
	apcFroms.Init();
	pObject1->GetHeapFroms(&apcFroms);
	AssertSize(1, apcFroms.NumElements());
	AssertPointer(pRoot->TestGetSet(), (*apcFroms.Get(0)));

	AssertFalse(pObject1->IsDirty());
	AssertNull(&pObject2);
	AssertNull(&pObject3);
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHollowObjectMemoryIteration(void)
{
	CFileUtil					cFileUtil;
	Ptr<CTestObject>			pObject1;
	Ptr<CTestObject>			pObject2;
	Ptr<CTestObject>			pObject3;
	Ptr<CRoot>					pRoot;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	STestObjectFreedNotifier	sFreeNotifider1;
	STestObjectFreedNotifier	sFreeNotifider2;
	STestObjectFreedNotifier	sFreeNotifider3;
	CArrayBaseObjectPtrE32		apcFroms;
	SIndexesIterator			sIter;
	OIndex						oi;
	CPointer					p;
	OIndex						oiRoot;
	OIndex						oiSet;
	OIndex						oi1;
	char						szDirectory[] = "Output" _FS_ "HollowObject2";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	TestHollowObjectAddConstructors();

	pRoot = ORoot();
	pObject1 = ONMalloc<CTestObject>("Just", &sFreeNotifider1);
	pObject2 = ONMalloc<CTestObject>("In", &sFreeNotifider2);
	pObject3 = ONMalloc<CTestObject>("Time", &sFreeNotifider3);
	pRoot->Add(pObject1);
	pObject1->mpTest = pObject2;
	pObject2->mpTest = pObject3;

	oiRoot = pRoot.GetIndex();
	oiSet = pRoot->TestGetSet()->GetIndex();
	oi1 = pObject1.GetIndex();

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	AssertTrue(sFreeNotifider1.bFreed);
	AssertTrue(sFreeNotifider2.bFreed);
	AssertTrue(sFreeNotifider3.bFreed);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	pRoot = ORoot();
	pRoot->TouchAll();

	oi = gcObjects.StartMemoryIteration(&sIter);
	AssertLong(oiRoot, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(oiSet, oi);
	oi = gcObjects.IterateMemory(&sIter);
	AssertLong(oi1, oi);
	p = gcObjects.TestGetFromMemory(oi);
	AssertTrue(p.IsHollow());
	oi = gcObjects.IterateMemory(&sIter);
	sIter.Kill();
	AssertLong(INVALID_O_INDEX, oi);
	AssertString("CTestObject", p->ClassName());
	AssertFalse(p.IsHollow());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestHollowObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestHollowObjectAllocation();
	TestHollowObjectMemoryIteration();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

