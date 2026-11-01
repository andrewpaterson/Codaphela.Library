#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "TestLib/Assert.h"
#include "TestBaseObject.h"
#include "ObjectTestSetup.h"
#include "EmbeddedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupEmbeddedObjectConstructors(void)
{
	gcObjects.AddConstructor<CEmbeddedTest>();
	gcObjects.AddConstructor<CEmbeddedContainer>();
	gcObjects.AddConstructor<CEmbeddedComplex>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedFlags(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;

	AssertFalse(cComplex.HasClass());
	AssertFalse(cComplex.ma.HasClass());
	AssertFalse(cComplex.mcSimple.HasClass());
	AssertFalse(cComplex.mcContainer.HasClass());
	AssertFalse(cComplex.mcContainer.mcOne.HasClass());
	AssertFalse(cComplex.mcContainer.mcTwo.HasClass());
	AssertFalse(cComplex.IsInitialised());
	AssertFalse(cComplex.ma.IsInitialised());
	AssertFalse(cComplex.mcSimple.IsInitialised());
	AssertFalse(cComplex.mcContainer.IsInitialised());
	AssertFalse(cComplex.mcContainer.mcOne.IsInitialised());
	AssertFalse(cComplex.mcContainer.mcTwo.IsInitialised());

	cComplex.Init();

	AssertTrue(cComplex.HasClass());
	AssertTrue(cComplex.ma.HasClass());
	AssertTrue(cComplex.mcSimple.HasClass());
	AssertTrue(cComplex.mcContainer.HasClass());
	AssertTrue(cComplex.mcContainer.mcOne.HasClass());
	AssertTrue(cComplex.mcContainer.mcTwo.HasClass());
	AssertTrue(cComplex.IsInitialised());
	AssertTrue(cComplex.ma.IsInitialised());
	AssertTrue(cComplex.mcSimple.IsInitialised());
	AssertTrue(cComplex.mcContainer.IsInitialised());
	AssertTrue(cComplex.mcContainer.mcOne.IsInitialised());
	AssertTrue(cComplex.mcContainer.mcTwo.IsInitialised());

	cComplex.Kill();

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectAddDistToRoot(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CGameWorld>			pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile>	pMissile = ONMalloc<CClusterMissile>("Frank", pWorld);
	Ptr<CHarrier>			pHarrier = OMalloc<CHarrier>(pWorld);
	
	pWorld->AddPlayer(pHarrier);
	AssertInt(3, pHarrier->GetDistToRoot());

	pRoot->Add(pMissile);

	AssertInt(2, pMissile->GetDistToRoot());
	AssertInt(2, pMissile->mcMissile1.GetDistToRoot());
	AssertInt(2, pMissile->mcMissile2.GetDistToRoot());

	pMissile->mcMissile1.SetTarget(pHarrier);

	AssertInt(3, pHarrier->GetDistToRoot());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectRemoveDistToRoot(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CGameWorld>			pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile>	pMissile = ONMalloc<CClusterMissile>("Frank", pWorld);
	Ptr<CHarrier>			pHarrier = OMalloc<CHarrier>(pWorld);
	Ptr<CMissile>			pHolder = OMalloc<CMissile>(pWorld);
	
	pWorld->AddTickable(pHolder);
	AssertInt(4, pHolder->GetDistToRoot());

	pHolder->SetTarget(pHarrier);
	AssertInt(5, pHarrier->GetDistToRoot());

	Ptr<CClusterLauncher>	pLauncher = OMalloc<CClusterLauncher>();

	pRoot->Add(pLauncher);
	AssertInt(2, pLauncher->GetDistToRoot());

	pLauncher->mpMissile = pMissile;

	AssertInt(3, pMissile->GetDistToRoot());
	AssertInt(3, pMissile->mcMissile1.GetDistToRoot());
	AssertInt(3, pMissile->mcMissile2.GetDistToRoot());
	AssertInt(5, pHarrier->GetDistToRoot());
	AssertSize(1, pHarrier->NumHeapFroms());

	pMissile->mcMissile1.SetTarget(pHarrier);
	AssertInt(4, pHarrier->GetDistToRoot());
	AssertSize(2, pHarrier->NumHeapFroms());

	pWorld->RemoveTickable(pHolder);
	pHolder = NULL;

	AssertInt(4, pHarrier->GetDistToRoot());
	AssertSize(1, pHarrier->NumHeapFroms());

	pRoot->Add(pHarrier);
	AssertInt(2, pHarrier->GetDistToRoot());

	pRoot->Remove(pHarrier);
	AssertInt(4, pHarrier->GetDistToRoot());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKill(void)
{
	char	szClusterMissileState[64];
	char	szMissile1State[64];

	ObjectsInit();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CGameWorld> pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile> pClusterMissile = ONMalloc<CClusterMissile>("Anna", pWorld);
	pWorld = NULL;

	Ptr<CPointerContainer> pPointerPointer = OMalloc<CPointerContainer>(&pClusterMissile->mcMissile1);
	pRoot->Add(pPointerPointer);

	Ptr<CPointerContainer> pPointerPointer2 = OMalloc<CPointerContainer>(&pClusterMissile);
	pRoot->Add(pPointerPointer2);

	//The number of allocated object shouldn't change until all the froms are removed
	//both from the embedded object and the 'normal' object.
	AssertLong(7, gcObjects.GetMemory()->NumIndexed());
	AssertLong(7, gcUnknowns.NumElements());

	pClusterMissile->SetKillString(szClusterMissileState);
	pClusterMissile->mcMissile1.SetKillString(szMissile1State);

	AssertSize(2, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());
	strcpy(szClusterMissileState, "Alive");
	strcpy(szMissile1State, "Alive");

	pPointerPointer->Clear();

	AssertSize(1, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(0, pClusterMissile->mcMissile1.NumHeapFroms());
	AssertString("Alive", szClusterMissileState);
	AssertString("Alive", szMissile1State);

	pPointerPointer->mp = &pClusterMissile->mcMissile1;

	AssertSize(2, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());

	pPointerPointer2->Clear();

	AssertSize(1, pClusterMissile->NumHeapFroms());
	AssertSize(0, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());
	AssertString("Alive", szClusterMissileState);
	AssertString("Alive", szMissile1State);

	pClusterMissile = NULL;

	//Make sure nothing has been de-allocated.
	AssertLong(7, gcObjects.GetMemory()->NumIndexed());
	AssertLong(7, gcUnknowns.NumElements());

	pPointerPointer->Clear();

	AssertString("Killed", szClusterMissileState);
	AssertString("Killed", szMissile1State);

	AssertLong(6, gcObjects.GetMemory()->NumIndexed());
	AssertLong(6, gcUnknowns.NumElements());

	ObjectsFlush();
	gcObjects.EvictInMemory();

	AssertLong(0, gcObjects.GetMemory()->NumIndexed());
	AssertLong(0, gcUnknowns.NumElements());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectContainerDehollowfication(void)
{
	bool			bResult;
	CFileUtil		cFileUtil;
	const char*		szClassName;
	OIndex			oiComplex;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	size			uiEmbeddedTestSize;
	size			uiEmbeddedContainerSize;
	char			szDirectory[] = "Output" _FS_ "EmbeddedObject";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CEmbeddedComplex> pComplex = OMalloc<CEmbeddedComplex>();
	oiComplex = pComplex->GetIndex();

	pRoot->Add(pComplex);

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	uiEmbeddedTestSize = OBJECT_SIZE + sizeof(int32) + sizeof(float) + POINTER_SIZE;
	AssertLayoutSize(sizeof(CEmbeddedTest), uiEmbeddedTestSize);
	uiEmbeddedContainerSize = OBJECT_SIZE + 4 + uiEmbeddedTestSize + sizeof(int32) + uiEmbeddedTestSize + sizeof(float) + POINTER_SIZE + 12  /* random padding I assume */;
	AssertLayoutSize(sizeof(CEmbeddedContainer), uiEmbeddedContainerSize);
	AssertLayoutSize(sizeof(CEmbeddedComplex), OBJECT_SIZE + uiEmbeddedTestSize + sizeof(int32) * 2 + sizeof(CArray<>) + uiEmbeddedContainerSize + POINTER_SIZE);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	pRoot = gcObjects.Root();
	AssertTrue(pRoot.IsNotNull());
	pComplex = pRoot->GetFirst<CEmbeddedComplex>();
	AssertTrue(pComplex.IsHollow());
	
	szClassName = pComplex->ClassName();
	AssertString("CEmbeddedComplex", szClassName);
	AssertLong(oiComplex, pComplex.GetIndex());

	AssertInt((int32)1, pComplex->mai[0]);
	AssertInt((int32)2, pComplex->mai[1]);
	
	AssertLong(INVALID_O_INDEX, pComplex->mcSimple.GetIndex());
	AssertInt((int32)85, pComplex->mcSimple.miAmANumber);
	AssertFloat(58.0f, pComplex->mcSimple.mfSoAmI, 0);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.GetIndex());
	AssertInt((int32)73, pComplex->mcContainer.mi);
	AssertFloat(666.0f, pComplex->mcContainer.mf, 0);
	AssertString("And", pComplex->mcContainer.msz);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.mcOne.GetIndex());
	AssertInt((int32)85, pComplex->mcContainer.mcOne.miAmANumber);
	AssertFloat(58.0f, pComplex->mcContainer.mcOne.mfSoAmI, 0);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.mcTwo.GetIndex());
	AssertInt((int32)85, pComplex->mcContainer.mcTwo.miAmANumber);
	AssertFloat(58.0f, pComplex->mcContainer.mcTwo.mfSoAmI, 0);

	AssertLong(3, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectPointTo(void)
{
	bool		bResult;
	CFileUtil	cFileUtil;
	const char*	szClassName;
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	char		szDirectory[] = "Output" _FS_ "EmbeddedObject";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CEmbeddedComplex> pComplex = OMalloc<CEmbeddedComplex>();

	Ptr<CEmbeddedContainer> pContainer = &pComplex->mcContainer;
	pRoot->Add(pContainer);

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	AssertNull(&pContainer);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	pRoot = gcObjects.Root();
	AssertTrue(pRoot.IsNotNull());
	pContainer = pRoot->GetFirst<CEmbeddedContainer>();
	AssertTrue(pContainer.IsHollow());
	AssertSize(0, pContainer.Object()->NumEmbedded());
	AssertLong(-1, pContainer.GetIndex());

	szClassName = pContainer->ClassName();
	AssertString("CEmbeddedContainer", szClassName);

	pComplex = pContainer->GetEmbeddingContainer();
	//Kinda feels like this test just stopped...

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedGetEmbeddedIndex(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;

	cComplex.Init();

	AssertSize(0, cComplex.TestGetNumEmbeddedFromFlags());
	AssertSize(6, cComplex.NumEmbedded());
	AssertSize(6, cComplex.TestGetNumEmbeddedFromFlags());

	AssertSize(1, cComplex.mcSimple.NumEmbedded());
	AssertSize(1, cComplex.ma.NumEmbedded());
	AssertSize(3, cComplex.mcContainer.NumEmbedded());
	AssertSize(1, cComplex.mcContainer.mcOne.NumEmbedded());
	AssertSize(1, cComplex.mcContainer.mcOne.NumEmbedded());

	AssertSize(0, cComplex.GetEmbeddedIndex(&cComplex));
	AssertSize(1, cComplex.GetEmbeddedIndex(&cComplex.mcSimple));
	AssertSize(2, cComplex.GetEmbeddedIndex(&cComplex.ma));
	AssertSize(3, cComplex.GetEmbeddedIndex(&cComplex.mcContainer));
	AssertSize(4, cComplex.GetEmbeddedIndex(&cComplex.mcContainer.mcOne));
	AssertSize(5, cComplex.GetEmbeddedIndex(&cComplex.mcContainer.mcTwo));
	AssertSize(-1, cComplex.GetEmbeddedIndex(NULL));

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedGetEmbeddedObject(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;
	CEmbeddedObject*	pcEmbedded;

	cComplex.Init();

	pcEmbedded = cComplex.GetEmbeddedObject(0);
	AssertPointer(&cComplex, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(1);
	AssertPointer(&cComplex.mcSimple, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(2);
	AssertPointer(&cComplex.ma, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(3);
	AssertPointer(&cComplex.mcContainer, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(4);
	AssertPointer(&cComplex.mcContainer.mcOne, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(5);
	AssertPointer(&cComplex.mcContainer.mcTwo, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(6);
	AssertNull(pcEmbedded);

	pcEmbedded = cComplex.mcSimple.GetEmbeddedObject(0);
	AssertPointer(&cComplex.mcSimple, pcEmbedded);
	pcEmbedded = cComplex.mcSimple.GetEmbeddedObject(1);
	AssertNull(pcEmbedded);

	pcEmbedded = cComplex.ma.GetEmbeddedObject(0);
	AssertPointer(&cComplex.ma, pcEmbedded);
	pcEmbedded = cComplex.ma.GetEmbeddedObject(1);
	AssertNull(pcEmbedded);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectClass(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;
	CPointer*			pcPtr0;
	CPointer*			pcPtr1;
	CPointer*			pcPtr2;
	CPointer*			pcPtr3;
	CPointer*			pcPtr4;
	CPointer*			pcPtr5;

	//Class not yet called.  Always call Class.
	AssertSize(0, cComplex.GetNumFieldPointerTos());
	AssertSize(1, cComplex.NumEmbedded());

	cComplex.ClearFlagNumEmbedded();
	cComplex.Init();
	AssertSize(5, cComplex.GetNumFieldPointerTos());
	AssertSize(6, cComplex.NumEmbedded());

	pcPtr0 = cComplex.GetFieldPointerTo(0);
	pcPtr1 = cComplex.GetFieldPointerTo(1);
	pcPtr2 = cComplex.GetFieldPointerTo(2);
	pcPtr3 = cComplex.GetFieldPointerTo(3);
	pcPtr4 = cComplex.GetFieldPointerTo(4);
	pcPtr5 = cComplex.GetFieldPointerTo(5);

	AssertPointer(cComplex.mpTest.This(), pcPtr0);
	AssertPointer(cComplex.mcSimple.mpTest.This(), pcPtr1);
	AssertPointer(cComplex.mcContainer.mpTest.This(), pcPtr2);
	AssertPointer(cComplex.mcContainer.mcOne.mpTest.This(), pcPtr3);
	AssertPointer(cComplex.mcContainer.mcTwo.mpTest.This(), pcPtr4);
	AssertNull(pcPtr5);

	AssertSize(0, cComplex.GetFieldPointerToIndex(pcPtr0));
	AssertSize(1, cComplex.GetFieldPointerToIndex(pcPtr1));
	AssertSize(2, cComplex.GetFieldPointerToIndex(pcPtr2));
	AssertSize(3, cComplex.GetFieldPointerToIndex(pcPtr3));
	AssertSize(4, cComplex.GetFieldPointerToIndex(pcPtr4));
	AssertSize(-1, cComplex.GetFieldPointerToIndex(NULL));

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedOjectIsAllocatedInObjects()
{
	ObjectsInit();

	Ptr<CClusterMissile> pClusterMissile = ONMalloc<CClusterMissile>("Gerbil", Null());
	AssertTrue(pClusterMissile->IsAllocatedInObjects());
	AssertTrue(pClusterMissile->mcMissile1.IsAllocatedInObjects());
	AssertTrue(pClusterMissile->mcMissile2.IsAllocatedInObjects());
	pClusterMissile = NULL;

	CClusterMissile cClusterMissile;
	cClusterMissile.Init(NULL);
	AssertFalse(cClusterMissile.IsAllocatedInObjects());
	AssertFalse(cClusterMissile.mcMissile1.IsAllocatedInObjects());
	AssertFalse(cClusterMissile.mcMissile2.IsAllocatedInObjects());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKillSanity(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>		pContainer;
		Ptr<CEmbeddedTest>			pFieldOne;
		Ptr<CEmbeddedTest>			pFieldTwo;

		pContainer = OMalloc<CEmbeddedContainer>();

		pFieldOne = pContainer->GetOne();
		pFieldTwo = pContainer->GetTwo();

		AssertPointer(&pContainer->mcOne, &pFieldOne);
		AssertPointer(&pContainer->mcTwo, &pFieldTwo);

		pContainer->Kill();

		AssertNull(&pFieldOne);
		AssertNull(&pFieldTwo);
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKillFreesEmbedded(void)
{
	STestObjectFreedNotifier	sNotifierContainer;
	STestObjectFreedNotifier	sNotifierOne;
	STestObjectFreedNotifier	sNotifierTwo;
	STestObjectFreedNotifier	sNotifierThree;
	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;

	ObjectsInit();

	{
		CEmbeddedContainerWithNotifier		cContainer;
		Ptr<CEmbeddedTestWithNotifier>		pFieldOne;
		Ptr<CEmbeddedTestWithNotifier>		pFieldTwo;
		Ptr<CEmbeddedTestWithNotifier>		pFieldThree;

		Ptr<CTestObject>					pTestA;
		Ptr<CTestObject>					pTestB;
		Ptr<CTestObject>					pTestC;


		CTestObject							cTestD;
		CTestObject							cTestE;
		CTestObject							cTestF;

		pTestA = ONMalloc<CTestObject>("Aye", &sNotifierA);
		pTestB = ONMalloc<CTestObject>("Bee", &sNotifierB);
		pTestC = ONMalloc<CTestObject>("Sea", &sNotifierC);

		cContainer.Init(&sNotifierContainer,
			NULL,
			pTestA,
			pTestB,
			pTestC,
			&sNotifierOne,
			&sNotifierTwo,
			&sNotifierThree);

		pFieldOne = cContainer.GetOne();
		pFieldTwo = cContainer.GetTwo();
		pFieldThree = cContainer.GetThree();

		cTestD.Init(pFieldOne, NULL);
		cTestE.Init(pFieldTwo, NULL);
		cTestF.Init(pFieldThree, NULL);

		AssertPointer(&cContainer.mcOne, &pFieldOne);
		AssertPointer(&cContainer.mcTwo, &pFieldTwo);
		AssertPointer(&cContainer.mcThree, &pFieldThree);

		AssertFalse(sNotifierContainer.bFreed);
		AssertFalse(sNotifierOne.bFreed);
		AssertFalse(sNotifierTwo.bFreed);
		AssertFalse(sNotifierThree.bFreed);
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierB.bFreed);
		AssertFalse(sNotifierC.bFreed);

		cContainer.Kill();

		AssertTrue(sNotifierContainer.bFreed);
		AssertTrue(sNotifierOne.bFreed);
		AssertTrue(sNotifierTwo.bFreed);
		AssertTrue(sNotifierThree.bFreed);
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierB.bFreed);
		AssertFalse(sNotifierC.bFreed);

		AssertNull(&pFieldOne);
		AssertNull(&pFieldTwo);
		AssertNull(&pFieldThree);
		AssertNull(&cTestD.mpObject);
		AssertNull(&cTestE.mpObject);
		AssertNull(&cTestF.mpObject);
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectPointedToNoRemove(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierD;
	{
		Ptr<CTestObject>			pTestA;
		CTestObject					cTestD;

		pTestA = ONMalloc<CTestObject>("Aye", &sNotifierA);
		pTestA->mi = 0xA;

		cTestD.Init(&sNotifierD);
		cTestD.mi = 0xD;
		pTestA->mpObject = &cTestD;

		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertPointer(&cTestD, pTestA->mpObject.Object());
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierD.bFreed);

		pTestA = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifierA.bFreed);
		AssertFalse(sNotifierD.bFreed);
	}
	AssertTrue(sNotifierA.bFreed);
	AssertTrue(sNotifierD.bFreed);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbedded(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestEmbeddedFlags();
	TestEmbeddedOjectIsAllocatedInObjects();
	TestEmbeddedObjectAddDistToRoot();
	TestEmbeddedObjectRemoveDistToRoot();
	TestEmbeddedObjectKill();
	TestEmbeddedObjectContainerDehollowfication();
	TestEmbeddedGetEmbeddedIndex();
	TestEmbeddedObjectClass();
	TestEmbeddedGetEmbeddedObject();
	TestEmbeddedObjectPointTo();
	TestEmbeddedObjectKillSanity();
	TestEmbeddedObjectKillFreesEmbedded();
	TestEmbeddedObjectPointedToNoRemove();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

